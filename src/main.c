#include <acpi/table.h>
#include <asm/control.h>
#include <asm/io.h>
#include <asm/registers.h>
#include <boot/bootinfo.h>
#include <cpu/state.h>
#include <efi/table.h>
#include <gcc/interrupt.h>
#include <graphics/framebuffer.h>
#include <graphics/glyph.h>
#include <graphics/print.h>
#include <interrupt/apic.h>
#include <interrupt/idt.h>
#include <interrupt/pic.h>
#include <memory/manipulate.h>
#include <memory/memory_map.h>
#include <memory/page_allocator.h>
#include <memory/simple_allocator.h>
#include <paging/paging.h>
#include <ps2/controller.h>
#include <ps2/keyboard.h>
#include <segment/gdt.h>
#include <segment/tss.h>

extern char __kernel_data_begin[];
extern char __kernel_data_end[];

extern char __ap_init_begin[];
extern char __ap_init_end[];

static void kernel_loop()
{
    while (1);
}

static inline void ALWAYS_INLINE getbuff(char *buff, uint8_t data)
{
    buff[0] = '0';
    buff[1] = 'x';
    uint8_t first = (data >> 4) & 0xF;
    uint8_t second = data & 0xF;
    buff[2] = first >= 10 ? ('A' + first - 10) : ('0' + first);
    buff[3] = second >= 10 ? ('A' + second - 10) : ('0' + second);
    buff[4] = '\0';
}

static void parse_efi_system_table(BootInfo *bootInfo, uint8_t *core_ids, uint8_t *count, uint64_t *apic_addr)
{
    uint64_t addr = PAGING_ALIGN(bootInfo->efi_system_table);
    scratchpad_memory_map(0, addr, 5, 0);
    EFI_SYSTEM_TABLE *st = (EFI_SYSTEM_TABLE *) PAGING_PAGE_OFFSET(bootInfo->efi_system_table); 
    EFI_CONFIGURATION_TABLE *cfg = (EFI_CONFIGURATION_TABLE *) PAGING_PAGE_OFFSET(st->ConfigurationTable);
    EFI_GUID acpi_guid = ACPI_TABLE_GUID;
    uint64_t root_sys_desc = 0;
    for (uint64_t i = 0; i < st->NumberOfTableEntries && !root_sys_desc; i++)
    {
        if (memeq(&acpi_guid, &cfg->VendorGuid, sizeof(EFI_GUID)))
            root_sys_desc = (uint64_t) cfg->VendorTable;
        cfg++;
    }

    if (root_sys_desc)
    {
        addr = PAGING_ALIGN(root_sys_desc);
        scratchpad_memory_map(0, addr, 1, 0);
        acpi_rsdp *rsdp = (acpi_rsdp *) PAGING_PAGE_OFFSET(root_sys_desc);
        if (acpi_validate_rsdp(rsdp))
        {
            addr = PAGING_ALIGN(rsdp->rsdtaddress);
            acpi_rsdt *rsdt = (acpi_rsdt *) PAGING_PAGE_OFFSET(rsdp->rsdtaddress);
            scratchpad_memory_map(0, addr, 1, 0);
            if (acpi_validate_rsdt(rsdt))
            {
                uint32_t length;
                uint64_t addr = acpi_1_get_table_pointer(rsdt, ACPI_SIGNATURE_MADT, &length);
                if (addr)
                {
                    uint64_t page_count = get_page_count(length);
                    scratchpad_memory_map(0, PAGING_ALIGN(addr), page_count, 0);
                    acpi_madt *madt = (acpi_madt *) PAGING_PAGE_OFFSET(addr);
                    acpi_madt_get_processors(madt, core_ids, count, apic_addr); 
                }
            }
        }
    }
}

static mem_map *init_mem_map(BootInfo *bootInfo, simple_allocator *data_alloc)
{
    uint64_t rb_alloc_size = sizeof(simple_allocator) + bootInfo->memorymap.size;
    uint64_t mem_alloc_size = sizeof(simple_allocator) + (bootInfo->memorymap.size >> 1);
    mem_map *map = simple_allocator_alloc(data_alloc, sizeof(mem_map));
    void *mem_alloc_addr = simple_allocator_alloc(data_alloc, mem_alloc_size);
    void *rb_alloc_addr = simple_allocator_alloc(data_alloc, rb_alloc_size);
    simple_allocator *mem_alloc = simple_allocator_init(mem_alloc_addr, mem_alloc_size);
    simple_allocator *rb_alloc = simple_allocator_init(rb_alloc_addr, rb_alloc_size);
    uint8_t map_res = memory_map_parse(bootInfo, rb_alloc, mem_alloc, map);
    simple_allocator_free(data_alloc, rb_alloc_size);
    if (!map_res)
        kernel_loop();
    return map;
}

BootInfo *boot;

void double_except()
{
    scratchpad_memory_map(0, boot->framebuffer.base, 1, 0);
    uint32_t *p = 0;
    for (uint32_t i = 0;i < 100;i++) {
        *p = 0xFF0000;
        p++;
    }
    kernel_loop();
}

void INTERRUPT page_fault(no_priv_change_frame *frame, uint64_t error_code)
{
    scratchpad_memory_map(0, boot->framebuffer.base, 1, 0);
    uint32_t *p = 0;
    for (uint64_t i = 0;i < 100; i++) {
        *p = 0xFF;
        p++;
    }
    kernel_loop();
}

static void init_cpu_state(mem_map *map, cpu_state *state)
{
    memset(state, 0, sizeof(cpu_state));
    state->gdt = (void *) 128;
    state->tss = (void *) 256;
    state->idt = (void *) 0x1000;
    state->ist_count = 1; 
    state->ist[0] = 0x2000;
    state->io_map_base = sizeof(task_state_segment);

    uint64_t flags = get_rflags();
    flags &= ~((1 << 13) | (1 << 12));
    set_rflags(flags);

    void *addr = 0;
    for (uint64_t i = 0; i < map->length; i++)
    {
        mem_region *reg = &map->map[i];
        uint64_t length = ((reg->end - reg->start) >> PAGING_PAGE_SIZE_EXP) + 1;
        if (length > 4) {
            addr = (void *) reg->start;
            reg->start += 4 * PAGING_PAGE_SIZE;
            break;
        }
    }
    if (!addr)
        kernel_loop();
    scratchpad_memory_map(0, (uint64_t) addr, 4, 0);

    segment_fill_gdt(state);
    segment_set_gdt(state);
    segment_fill_tss(state); 
    tss_fill(state);
    ltr(0x28);
    memset(state->idt, 0, PAGING_PAGE_SIZE);
    lidt(state->idt, PAGING_PAGE_SIZE - 1); 
    
    idt_register_handler(state->idt, 8, double_except, 1, 0, 0);
    idt_register_handler(state->idt, 14, (idt_handler) page_fault, 1, 0, 0);
}

static uint8_t place_ap_init_code(mem_map *map)
{
    const uint64_t mask = 0xFF000;
    const uint64_t mask1 = ~0xFFFFF;
    uint64_t addr = 0;
    for (uint64_t i = 0; i < map->length; i++)
    {
        mem_region *reg = &map->map[i];
        if (reg->start < 0x100000)
        {
            addr = reg->start;
            break;
        }
    }
    if (!addr)
        return 0; 
    uint64_t size = (uint8_t *) __ap_init_end - (uint8_t *) __ap_init_begin;
    scratchpad_memory_map(0, addr, 1, 0);
    memset(0, 0, PAGING_PAGE_SIZE);
    memcpy(0, (void *) __ap_init_begin, size);
    
    return (uint8_t) ((addr & mask) >> PAGING_PAGE_SIZE_EXP);
}

static page_allocator *init_page_alloc(BootInfo *bootInfo, mem_map *map, simple_allocator *data_alloc)
{
    uint64_t pg_alloc_size = sizeof(simple_allocator) + bootInfo->memorymap.size;
    void *pg_alloc_addr = simple_allocator_alloc(data_alloc, pg_alloc_size);
    simple_allocator *pg_alloc = simple_allocator_init(pg_alloc_addr, pg_alloc_size);
    if (!pg_alloc)
        kernel_loop();
    page_allocator *page_alloc = page_allocator_init(map, pg_alloc);
    simple_allocator_free(data_alloc, pg_alloc_size);
    if (!page_alloc)
        kernel_loop();
    return page_alloc;
}

static graphics_glyph_description *init_graphics(BootInfo *bootInfo, page_allocator *page_alloc, simple_allocator *data_alloc)
{
    graphics_glyph_description *desc = simple_allocator_alloc(data_alloc, sizeof(graphics_glyph_description));
    graphics_framebuffer_init(bootInfo, &desc->framebuffer, page_alloc); 
    desc->glyph_vaddr = 0x18FFE000; 
    desc->width = *((uint8_t *) desc->glyph_vaddr);
    desc->height = *((uint8_t *) desc->glyph_vaddr + 1);
    return desc;
}

static uint8_t start_ap_cores(uint8_t ap_vector, uint8_t *core_ids, uint8_t count, uint64_t apic_addr)
{
    scratchpad_memory_map(0x4000, apic_addr, 1, 3);
    pic_disable();

    uint8_t apic_id = apic_get_id();
    apic_mask_all(0x4000);
    apic_enable(0x4000);

    uint64_t size = __ap_init_end - __ap_init_begin;
    scratchpad_memory_map(0x5000, (uint64_t) ap_vector << PAGING_PAGE_SIZE_EXP, 1, 3);
    volatile uint8_t *addr = (volatile uint8_t *)  (0x5000 + size - 1);
    *addr = 0;
    uint8_t prev = *addr;
    for (uint8_t i = 0; i < count; i++)
    {
        if (core_ids[i] == apic_id)
            continue; 
        apic_send_ipi(0x4000, core_ids[i], APIC_INIT_IPI, 0);
        apic_ipi_wait(0x4000);
        for (uint16_t j = 0; j < 10000; j++)
            io_wait();
        apic_send_ipi(0x4000, core_ids[i], APIC_STARTUP_IPI, ap_vector);
        apic_ipi_wait(0x4000);
        for (uint16_t j = 0; j < 10000; j++)
            io_wait();
        if (*addr == prev) {
            apic_send_ipi(0x4000, core_ids[i], APIC_STARTUP_IPI, ap_vector);
            apic_ipi_wait(0x4000);
            for (uint16_t j = 0; j < 10000; j++)
                io_wait();
        }
        prev = *addr;
    }
    
    return prev;
}

static void keyboard_init(cpu_state *state, graphics_glyph_description *glyph_desc)
{
    pic_init(0x20, 0x28); 
    graphics_glyph_color color;
    color.bg_red = 0;
    color.bg_green = 0;
    color.bg_blue = 0;
    color.fg_red = 255;
    color.fg_blue = 0;
    color.fg_green = 0;
    uint8_t stat = ps2_controller_init();
    if (stat == PS2_NOT_A_KEYBOARD) 
    {
        graphics_print_string(glyph_desc, "PS/2 - not a keyboard", 4, 0, &color);
        kernel_loop();
    } 
    else if (stat == PS2_DISABLE_SCAN_FAIL)
    {
        graphics_print_string(glyph_desc, "PS/2 - diable scan fail", 4, 0, &color);
        kernel_loop();
    }
    else if (stat == PS2_CONTROLLER_SELF_TEST_FAIL)
    {
        graphics_print_string(glyph_desc, "PS/2 - controller test fail", 4, 0, &color);
        kernel_loop();
    }
    else if (stat == PS2_PORT_TEST_FAILED)
    {
        graphics_print_string(glyph_desc, "PS/2 - port test fail", 4, 0, &color);
        kernel_loop();
    }
    else if (stat == PS2_RESET_FAILED)
    {
        graphics_print_string(glyph_desc, "PS/2 - device reset fail", 4, 0, &color);
        kernel_loop();
    }
    else if (stat == PS2_IDENTIFY_FAILED)
    {
        graphics_print_string(glyph_desc, "PS/2 - identify fail", 4, 0, &color);
        kernel_loop();
    }
    else if (stat == PS2_ENABLE_SCAN_FAIL)
    {
        graphics_print_string(glyph_desc, "PS/2 - enable scan fail", 4, 0, &color);
        kernel_loop();
    }
    color.fg_red = 0;
    color.fg_green = 255;
    ps2_keyboard_init(state->idt, 0x21);
    sti();
}

int kernel_main(BootInfo *bootInfo) 
{
    simple_allocator *data_alloc = simple_allocator_init((void *) __kernel_data_begin, __kernel_data_end - __kernel_data_begin); 
    paging_state *p_state = paging_init(get_cr3(), simple_allocator_alloc(data_alloc, sizeof(paging_state)));
    cpu_state *c_state = simple_allocator_alloc(data_alloc, sizeof(cpu_state));
    BootInfo *boot_info = simple_allocator_alloc(data_alloc, sizeof(BootInfo));
    uint8_t *core_count = simple_allocator_alloc(data_alloc, sizeof(uint8_t));
    uint64_t *apic_addr = simple_allocator_alloc(data_alloc, sizeof(uint64_t));
    uint8_t *core_ids = simple_allocator_alloc(data_alloc, 32 * sizeof(uint8_t));
    memcpy(boot_info, bootInfo, sizeof(BootInfo));
    boot = boot_info;
    
    parse_efi_system_table(bootInfo, core_ids, core_count, apic_addr);

    mem_map *map = init_mem_map(boot_info, data_alloc);
    
    uint8_t ap_vector = place_ap_init_code(map);
     
    init_cpu_state(map, c_state);

    uint8_t ok = start_ap_cores(ap_vector, core_ids, *core_count, *apic_addr);

    page_allocator *page_alloc = init_page_alloc(boot_info, map, data_alloc);

    graphics_glyph_description *glyph_desc = init_graphics(boot_info, page_alloc, data_alloc);
    graphics_glyph_color color;
    color.bg_red = 0;
    color.bg_green = 0;
    color.bg_blue = 0;
    color.fg_red = 0;
    color.fg_blue = 0;
    color.fg_green = 255;
    graphics_print_string(glyph_desc, "Welcome to SipOS!", 0, 0, &color);
    color.fg_red = 255;
    graphics_print_string(glyph_desc, "How do you get from point A to point B ?", 2, 0, &color);
    graphics_print_string(glyph_desc, "Easy! Just take an x-y plane or a rhombus.", 3, 0, &color);
    color.fg_blue = 255;
    char buffer[10];
    getbuff(buffer, *core_count);
    graphics_print_string(glyph_desc, "Cores discovered through ACPI MADT:", 4, 0, &color);
    graphics_print_string(glyph_desc, buffer, 5, 0, &color);

    getbuff(buffer, ok);
    graphics_print_string(glyph_desc, "Cores activated:", 6, 0, &color);
    graphics_print_string(glyph_desc, buffer, 7, 0, &color);
    
    keyboard_init(c_state, glyph_desc);

    color.fg_blue = 255;
    uint32_t row, col;
    row = 10;
    col = 0;
    ps2_key_event event;
    char buff[2];
    buff[1] = 0;
    char cursor[2] = "_";
    char nothing[2] = " ";
    uint32_t last[200];
    graphics_print_string(glyph_desc, cursor, row, col, &color);
    while (1)
    {
        ps2_keyboard_get_key(&event);
        if (event.released)
        {
            if (event.ascii)
            {
                buff[0] = event.ascii;
                graphics_print_string(glyph_desc, buff, row, col, &color);
                col++;
                if (col == 200) 
                {
                    if (row < 200)
                    {
                        last[row] = 199;
                        row++;
                        col = 0;
                    }
                    else
                        col--;
                }
                graphics_print_string(glyph_desc, cursor, row, col, &color);
            }
            else if (event.code == PS2_KEYBOARD_ENTER)
            {
                graphics_print_string(glyph_desc, nothing, row, col, &color);
                last[row] = col;
                row++;
                col = 0;
                graphics_print_string(glyph_desc, cursor, row, col, &color);
            }
            else if (event.code == PS2_KEYBOARD_BACKSPACE)
            {
                graphics_print_string(glyph_desc, nothing, row, col, &color);
                if (col > 0)
                    col--;
                else if (row > 9)
                {
                    row--;
                    col = last[row];
                }           
                graphics_print_string(glyph_desc, cursor, row, col, &color);
            } 
        }
    }

    kernel_loop();
    return 0;
}