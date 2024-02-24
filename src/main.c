#include <asm/control.h>
#include <asm/registers.h>
#include <boot/bootinfo.h>
#include <cpu/state.h>
#include <graphics/framebuffer.h>
#include <graphics/glyph.h>
#include <graphics/print.h>
#include <interrupt/idt.h>
#include <interrupt/pic.h>
#include <memory/manipulate.h>
#include <memory/memory_map.h>
#include <memory/page_allocator.h>
#include <memory/simple_allocator.h>
#include <paging/paging.h>
#include <segment/gdt.h>
#include <segment/tss.h>

extern char __kernel_data_begin[];
extern char __kernel_data_end[];

static void kernel_loop()
{
    while (1);
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

void double_except()
{
    kernel_loop();
}

void page_fault()
{
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
    pic_init(0x20, 0x28); 
    pic_clear_mask(2);

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
    scratchpad_memory_map(0, (uint64_t) addr, 4);

    segment_fill_gdt(state);
    segment_set_gdt(state);
    segment_fill_tss(state); 
    tss_fill(state);
    ltr(0x28);
    memset(state->idt, 0, PAGING_PAGE_SIZE);
    lidt(state->idt, PAGING_PAGE_SIZE - 1); 
    
    idt_register_handler(state->idt, 8, double_except, 1, 0, 0);
    idt_register_handler(state->idt, 14, page_fault, 1, 0, 0);
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

int kernel_main(BootInfo *bootInfo) 
{
    simple_allocator *data_alloc = simple_allocator_init((void *) __kernel_data_begin, __kernel_data_end - __kernel_data_begin); 
    paging_state *p_state = paging_init(get_cr3(), simple_allocator_alloc(data_alloc, sizeof(paging_state)));
    cpu_state *c_state = simple_allocator_alloc(data_alloc, sizeof(cpu_state));
    BootInfo *boot_info = simple_allocator_alloc(data_alloc, sizeof(BootInfo));
    memcpy(boot_info, bootInfo, sizeof(BootInfo));

    mem_map *map = init_mem_map(boot_info, data_alloc);

    init_cpu_state(map, c_state);

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
    
    kernel_loop();
    return 0;
}