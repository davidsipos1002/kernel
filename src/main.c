#include <asm/registers.h>
#include <boot/bootinfo.h>
#include <memory/manipulate.h>
#include <memory/memory_map.h>
#include <memory/page_allocator.h>
#include <memory/simple_allocator.h>
#include <paging/paging.h>
#include <segment/gdt.h>

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

static void init_cpu_data(mem_map *map, void **gdt, void **tss, void **idt)
{
    void *addr = 0;
    for (uint64_t i = 0; i < map->length; i++)
    {
        mem_region *reg = &map->map[i];
        uint64_t length = ((reg->end - reg->start) >> PAGING_PAGE_SIZE_EXP) + 1;
        if (length > 2) {
            addr = (void *) reg->start;
            reg->start += 2 * PAGING_PAGE_SIZE;
            break;
        }
    }
    if (!addr)
        kernel_loop();
    scratchpad_memory_map(0, (uint64_t) addr, 2);

    *gdt = 100;
    segment_fill_gdt(100);
    segment_set_gdt(100);
}

static page_allocator *init_page_alloc(BootInfo *bootInfo, mem_map *map, simple_allocator *data_alloc)
{
    uint64_t pg_alloc_size = sizeof(simple_allocator) + bootInfo->memorymap.size;
    void *pg_alloc_addr = simple_allocator_alloc(data_alloc, pg_alloc_size);
    simple_allocator *pg_alloc = simple_allocator_init(pg_alloc_addr, pg_alloc_size);
    if (!pg_alloc)
        kernel_loop();
    page_allocator *page_alloc = page_allocator_init(map, pg_alloc);
    if (!page_alloc)
        kernel_loop();
    return page_alloc;
}

int kernel_main(BootInfo *bootInfo) 
{
    simple_allocator *data_alloc = simple_allocator_init((void *) __kernel_data_begin, __kernel_data_end - __kernel_data_begin); 
    paging_state* state = paging_init(get_cr3(), simple_allocator_alloc(data_alloc, sizeof(paging_state)));
    memcpy(simple_allocator_alloc(data_alloc, sizeof(BootInfo)), bootInfo, sizeof(BootInfo));
    mem_map *map = init_mem_map(bootInfo, data_alloc);
    void *gdt, *tss, *idt;
    init_cpu_data(map, &gdt, &tss, &idt);
    // page_allocator *page_alloc = init_page_alloc(bootInfo, data_alloc, map);
    int a = 0;
    kernel_loop();
    return 0;
}