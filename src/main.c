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
extern char __ap_ljmp_instr[];
extern char __ap_init_ljmp[];
extern char __ap_init_gdt[];
extern char __ap_init_gdtr[];
extern char __ap_init_protected[];
extern char __ap_absljmp_instr[];
extern char __ap_absljmp[];
extern char __ap_ljmp64_instr[];
extern char __ap_ljmp64[];

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
    scratchpad_memory_map(0, addr, 5, 0, 1);
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
        scratchpad_memory_map(0, addr, 1, 0, 1);
        acpi_rsdp *rsdp = (acpi_rsdp *) PAGING_PAGE_OFFSET(root_sys_desc);
        if (acpi_validate_rsdp(rsdp))
        {
            addr = PAGING_ALIGN(rsdp->rsdtaddress);
            acpi_rsdt *rsdt = (acpi_rsdt *) PAGING_PAGE_OFFSET(rsdp->rsdtaddress);
            scratchpad_memory_map(0, addr, 1, 0, 1);
            if (acpi_validate_rsdt(rsdt))
            {
                uint32_t length;
                uint64_t addr = acpi_1_get_table_pointer(rsdt, ACPI_SIGNATURE_MADT, &length);
                if (addr)
                {
                    uint64_t page_count = get_page_count(length);
                    scratchpad_memory_map(0, PAGING_ALIGN(addr), page_count, 0, 1);
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
    scratchpad_memory_map(0, boot->framebuffer.base, 1, 0, 1);
    uint32_t *p = 0;
    for (uint32_t i = 0;i < 100;i++) {
        *p = 0xFF0000;
        p++;
    }
    kernel_loop();
}

void INTERRUPT page_fault(no_priv_change_frame *frame, uint64_t error_code)
{
    scratchpad_memory_map(0, boot->framebuffer.base, 1, 0, 1);
    uint32_t *p = 0;
    for (uint64_t i = 0;i < 100; i++) {
        *p = 0xFF;
        p++;
    }
    kernel_loop();
}

void INTERRUPT general_protection(no_priv_change_frame *frame, uint64_t error_code)
{
    scratchpad_memory_map(0, boot->framebuffer.base, 1, 0, 1);
    uint32_t *p = 0;
    for (uint64_t i = 0;i < 100; i++) {
        *p = 0xFF00FF;
        p++;
    }
    kernel_loop();
}

static void init_cpu_state(mem_map *map, cpu_state *state, uint8_t ap_vector)
{
    memset(state, 0, sizeof(cpu_state));
    state->gdt = (void *) 0x100080;
    state->tss = (void *) 0x100100;
    state->idt = (void *) 0x101000;
    state->ist_count = 7; 
    state->ist[0] = 0x104000;
    state->ist[1] = 0x106000;
    state->ist[2] = 0x108000;
    state->ist[3] = 0x10A000;
    state->ist[4] = 0x10C000;
    state->ist[5] = 0x10D000;
    state->ist[6] = 0x10F000;
    state->io_map_base = sizeof(task_state_segment);

    uint64_t flags = get_rflags();
    flags &= ~((1 << 13) | (1 << 12));
    set_rflags(flags);

    void *addr = 0;
    for (uint64_t i = 0; i < map->length; i++)
    {
        mem_region *reg = &map->map[i];
        uint64_t length = ((reg->end - reg->start) >> PAGING_PAGE_SIZE_EXP) + 1;
        if (length > 16 && reg->start != ((uint64_t) ap_vector << PAGING_PAGE_SIZE_EXP)) {
            addr = (void *) reg->start;
            reg->start += 16 * PAGING_PAGE_SIZE;
            break;
        }
    }
    if (!addr)
        kernel_loop();
    scratchpad_memory_map(0x100000, (uint64_t) addr, 16, 0, 1);

    segment_fill_gdt(state);
    segment_set_gdt(state);
    segment_fill_tss(state); 
    tss_fill(state);
    ltr(0x28);
    memset(state->idt, 0, PAGING_PAGE_SIZE);
    lidt(state->idt, PAGING_PAGE_SIZE - 1); 
    
    idt_register_handler(state->idt, 8, double_except, 2, 0, 0);
    idt_register_handler(state->idt, 13, (idt_handler) general_protection, 2, 0, 0);
    idt_register_handler(state->idt, 14, (idt_handler) page_fault, 1, 0, 0);
}

typedef struct
{
    uint32_t cr3;
    uint8_t gdt[10];
    uint8_t idt[10];
    uint64_t rsp;
    uint32_t vector;
    uint32_t ap_test;
    uint8_t count;
} PACKED_STRUCT ap_init_s;

static uint8_t place_ap_init_code(mem_map *map)
{
    const uint64_t mask = 0xFF000;
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
    scratchpad_memory_map(0, addr, 1, 0, 1);
    memset(0, 0, PAGING_PAGE_SIZE);
    memcpy(0, (void *) __ap_init_begin, size);

    uint32_t *gdt_base = (uint32_t *) (__ap_init_gdtr - __ap_init_begin + 2);
    *gdt_base = addr + __ap_init_gdt - __ap_init_begin;
    
    uint16_t *ljmp = (uint16_t *) (__ap_ljmp_instr - __ap_init_begin + 1);
    *ljmp = __ap_init_ljmp - __ap_init_begin;
    
    uint32_t *absljmp = (uint32_t *) (__ap_absljmp_instr - __ap_init_begin + 1);
    *absljmp = addr + __ap_absljmp - __ap_init_begin;
    
    uint32_t *ljmp64 = (uint32_t *) (__ap_ljmp64_instr - __ap_init_begin + 1);
    *ljmp64 = addr + __ap_ljmp64 - __ap_init_begin;

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
    desc->glyph_vaddr = 0x1F3FE000;
    desc->width = *((uint8_t *) desc->glyph_vaddr);
    desc->height = *((uint8_t *) desc->glyph_vaddr + 1);
    return desc;
}

static uint8_t start_ap_cores(uint8_t ap_vector, uint8_t *core_ids, uint8_t count, uint64_t apic_addr, void *ap_stacks)
{
    scratchpad_memory_map(0x110000, apic_addr, 1, 3, 1);
    pic_disable();

    uint8_t apic_id = apic_get_id();
    apic_mask_all(0x110000);
    apic_enable(0x110000);

    uint64_t size = __ap_init_end - __ap_init_begin;
    uint64_t ap_init_code = (uint64_t) ap_vector << PAGING_PAGE_SIZE_EXP;
    scratchpad_memory_map(ap_init_code, ap_init_code, 1, 0, 0);
    scratchpad_memory_map(0x111000, ap_init_code, 1, 0, 1);
    volatile ap_init_s *ap_init = (volatile ap_init_s *) (0x111000 + size - sizeof(ap_init_s));
    ap_init->cr3 = get_cr3();
    ap_init->count = 0;
    ap_init->vector = (uint32_t) ap_vector << PAGING_PAGE_SIZE_EXP;
    ap_init->ap_test = 0;
    uint8_t prev = 0;
    uint64_t exec_base = (uint64_t) ap_vector << PAGING_PAGE_SIZE_EXP;
    for (uint8_t i = 0; i < count; i++)
    {
        if (core_ids[i] == apic_id)
            continue; 
        segment_descriptor *desc = (segment_descriptor *) (__ap_init_gdt - __ap_init_begin);
        desc++;
        desc->base_0 = exec_base & 0xFFFF;
        desc->base_1 = (exec_base >> 16) & 0xF;
        desc++;
        desc->base_0 = exec_base & 0xFFFF;
        desc->base_1 = (exec_base >> 16) & 0xF;
        ap_init->rsp = ((uint64_t) ap_stacks) + 100 * prev * PAGING_PAGE_SIZE;
        apic_send_ipi(0x110000, core_ids[i], APIC_INIT_IPI, 0);
        apic_ipi_wait(0x110000);
        for (uint16_t j = 0; j < 10000; j++)
            io_wait();
        apic_send_ipi(0x110000, core_ids[i], APIC_STARTUP_IPI, ap_vector);
        apic_ipi_wait(0x110000);
        for (uint16_t j = 0; j < 10000; j++)
            io_wait();
        if (ap_init->count == prev) {
            apic_send_ipi(0x110000, core_ids[i], APIC_STARTUP_IPI, ap_vector);
            apic_ipi_wait(0x110000);
            for (uint16_t j = 0; j < 10000; j++)
                io_wait();
        }    
        prev = ap_init->count;
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
    void *ap_stacks = simple_allocator_alloc(data_alloc, 32 * PAGING_PAGE_SIZE * 100);
    memcpy(boot_info, bootInfo, sizeof(BootInfo));
    boot = boot_info;

    
    parse_efi_system_table(bootInfo, core_ids, core_count, apic_addr);
    mem_map *map = init_mem_map(boot_info, data_alloc);
    
    uint8_t ap_vector = place_ap_init_code(map);
     
    init_cpu_state(map, c_state, ap_vector);
    
    uint8_t ok = 0;
    ok = start_ap_cores(ap_vector, core_ids, *core_count, *apic_addr, ap_stacks);

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