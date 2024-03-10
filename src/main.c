#include <asm/control.h>
#include <asm/io.h>
#include <asm/registers.h>
#include <boot/bootinfo.h>
#include <cpu/core.h>
#include <cpu/state.h>
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
#include <sync/spinlock.h>

extern char __kernel_data_begin[];
extern char __kernel_data_end[];

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

typedef struct
{
    spinlock *lock;
    spinlock *done;
    uint64_t count;
} ap_param;

void ap_start(void *p)
{
    ap_param *param = (ap_param *) p;
    spinlock *lock = param->lock;
    spinlock *done = param->done;
    while(1)
    {
        spinlock_lock(lock);
        param->count++;
        spinlock_release(done);
    }
    hlt();
}

uint8_t start_cores(BootInfo *bootInfo, simple_allocator *alloc, mem_map *map, 
uint8_t *core_ids, uint8_t *count, uint8_t *ap_vector, uint64_t *apic_addr, void *ap_stacks, spinlock *locks, spinlock *done, ap_param *params)
{
    parse_efi_system_table(bootInfo, core_ids, count, apic_addr);
    *ap_vector = place_ap_init_code(map);
    uint8_t id = apic_get_id();
    ap_init *init = simple_allocator_alloc(alloc, (*count - 1) * sizeof(ap_init));
    uint8_t ndx = 0;
    for (uint8_t i = 0; i < *count; i++)
    {
        if (core_ids[i] == id)
            continue;
        init[ndx].core_id = core_ids[i];
        init[ndx].stack = (void *) (((uint64_t) ap_stacks) + 100 * (ndx + 1) * PAGING_PAGE_SIZE);
        init[ndx].start = ap_start;
        spinlock_lock(&locks[ndx]);
        spinlock_lock(&done[ndx]);
        params[ndx].lock = &locks[ndx];
        params[ndx].done = &done[ndx];
        params[ndx].count = 0;
        init[ndx].param = &params[ndx];
        ndx++;
    }
    uint8_t ret = start_ap_cores(*ap_vector, *apic_addr, init, *count);
    simple_allocator_free(alloc, (*count - 1) * sizeof(ap_init));
    return ret;
}

uint32_t parsenum(char *buff)
{
    uint32_t res = 0, count = 0, pow = 1;
    char *tmp = buff;
    while('0' <= *tmp && *tmp <= '9')
    {
        tmp++;
        count++;
    }
    if (count)
    {
        for (uint8_t i = 0; i < count - 1; i++)
            pow *= 10;
        for (uint8_t i = 0; i < count; i++)
        {
            res += (uint32_t) (buff[i] - '0') * pow;
            pow /= 10;
        }
    }
    return res;
}

void execmd(graphics_glyph_description *glyph_desc, char *buff, int row, uint8_t core_count, volatile ap_param *param)
{
    graphics_glyph_color color;
    color.bg_red = color.bg_green = color.bg_blue = 0;
    if (memeq(buff, "echo ", 5))
    {
        color.bg_red = color.bg_green = color.bg_blue = 0;
        color.fg_red = color.fg_green = color.fg_blue = 255;
        graphics_print_string(glyph_desc, buff + 5, row, 0, &color);
        return;
    }
    if (memeq(buff, "ping ", 5))
    {
        uint32_t n = parsenum(buff + 5);
        if (!core_count || n >= core_count - 1)
        {
            color.fg_red = 255;
            graphics_print_string(glyph_desc, "Invalid core!", row, 0, &color);
        }
        else
        { 
            color.bg_red = color.bg_green = color.bg_blue = 0;
            color.fg_red = color.fg_green = color.fg_blue = 255;
            spinlock_release(param[n].lock);
            spinlock_lock(param[n].done);
            char cntbuff[5];
            getbuff(cntbuff, param[n].count);
            graphics_print_string(glyph_desc, cntbuff, row, 0,&color);
        }
        return;
    }
    color.fg_red = 255;
    graphics_print_string(glyph_desc, "Unknown command!", row, 0, &color);
}

void shell(graphics_glyph_description *glyph_desc, uint8_t core_count, volatile ap_param *param)
{
    static char cmdbuff[201];
    static char empty[200];
    static int cmdndx;
    memset(empty, ' ', 200);
    uint32_t row, col;
    row = 6;
    col = 2;
    ps2_key_event event;
    char buff[2];
    buff[1] = 0;
    char cursor[2] = "_";
    char nothing[2] = " ";
    graphics_glyph_color color;
    color.bg_red = color.bg_green = color.bg_blue = 0;
    color.fg_red = color.fg_green = color.fg_blue = 255;
    graphics_print_string(glyph_desc, "> ", row, 0, &color);
    graphics_print_string(glyph_desc, cursor, row, col, &color);
    while (1)
    {
        ps2_keyboard_get_key(&event);
        if (event.event == 1)
        {
            if (event.ascii)
            {
                buff[0] = event.ascii;
                graphics_print_string(glyph_desc, buff, row, col, &color);
                cmdbuff[cmdndx++] = event.ascii;
                col++;
                if (col == 200) 
                    col--;
                graphics_print_string(glyph_desc, cursor, row, col, &color);
            }
            else if (event.code == PS2_KEYBOARD_ENTER)
            {
                graphics_print_string(glyph_desc, nothing, row, col, &color);
                cmdbuff[cmdndx] = 0;
                execmd(glyph_desc, cmdbuff, row + 1, core_count, param);
                cmdndx = 0;
                row += 2;
                col = 2;
                graphics_print_string(glyph_desc, "> ", row, 0, &color);
                graphics_print_string(glyph_desc, cursor, row, col, &color);
            }
            else if (event.code == PS2_KEYBOARD_BACKSPACE)
            {
                cmdndx--;
                graphics_print_string(glyph_desc, nothing, row, col, &color);
                if (col > 2)
                    col--;
                else if (row > 7)
                {
                    graphics_print_string(glyph_desc, "  ", row, 0, &color);
                    graphics_print_string(glyph_desc, empty, row - 1, 0, &color);
                    cmdndx = 0;
                    row -= 2;
                    graphics_print_string(glyph_desc, empty, row, 0, &color);
                    graphics_print_string(glyph_desc, "> ", row, 0, &color);
                    col = 2;
                }           
                graphics_print_string(glyph_desc, cursor, row, col, &color);
            } 
        }
    }

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
    simple_allocator_align(data_alloc, 4096);
    void *ap_stacks = simple_allocator_alloc(data_alloc, 32 * PAGING_PAGE_SIZE * 100);
    spinlock *core_locks = simple_allocator_alloc(data_alloc, 32 * sizeof(spinlock));
    spinlock *done_locks = simple_allocator_alloc(data_alloc, 32 * sizeof(spinlock));
    ap_param *params = simple_allocator_alloc(data_alloc, 32 * sizeof(ap_param)); 
    memset(core_locks, 0, 32 * sizeof(spinlock));
    memcpy(boot_info, bootInfo, sizeof(BootInfo));
    boot = boot_info;

    mem_map *map = init_mem_map(boot_info, data_alloc);

    
    uint8_t ap_vector, started;
    started = start_cores(boot_info, data_alloc, map, core_ids, 
        core_count, &ap_vector, apic_addr, ap_stacks, core_locks, done_locks, params);

    init_cpu_state(map, c_state, ap_vector);

    page_allocator *page_alloc = init_page_alloc(boot_info, map, data_alloc);

    graphics_glyph_description *glyph_desc = init_graphics(boot_info, page_alloc, data_alloc);
    graphics_glyph_color color;
    color.bg_red = color.bg_green = color.bg_blue = 0;
    color.fg_red = color.fg_blue = 0;
    color.fg_green = 255;
    graphics_print_string(glyph_desc, "Welcome to SipOS!", 0, 0, &color);
    color.fg_red = 255;
    char buffer[5];
    getbuff(buffer, *core_count);
    graphics_print_string(glyph_desc, "Cores discovered through ACPI MADT:", 1, 0, &color);
    graphics_print_string(glyph_desc, buffer, 2, 0, &color);

    getbuff(buffer, started);
    graphics_print_string(glyph_desc, "Cores activated:", 3, 0, &color);
    graphics_print_string(glyph_desc, buffer, 4, 0, &color);
    
    keyboard_init(c_state, glyph_desc);

    shell(glyph_desc, *core_count, params);
    
    kernel_loop();
    return 0;
}