#include <asm/registers.h>
#include <boot/bootinfo.h>
#include <memory/manipulate.h>
#include <memory/memory_map.h>
#include <memory/page_allocator.h>
#include <memory/simple_allocator.h>
#include <paging/paging.h>

extern char __kernel_data_begin[];
extern char __kernel_data_end[];

void kernel_loop()
{
    while (1);
}

int kernel_main(BootInfo *bootInfo) 
{
    simple_allocator *data_alloc = simple_allocator_init((void *) __kernel_data_begin, __kernel_data_end - __kernel_data_begin); 
    paging_state* state = paging_init(get_cr3(), simple_allocator_alloc(data_alloc, sizeof(paging_state)));
    memcpy(simple_allocator_alloc(data_alloc, sizeof(BootInfo)), bootInfo, sizeof(BootInfo));
    uint64_t rb_alloc_size = sizeof(simple_allocator) + bootInfo->memorymap.size;
    uint64_t mem_alloc_size = sizeof(simple_allocator) + (bootInfo->memorymap.size >> 1);
    mem_map *map = simple_allocator_alloc(data_alloc, sizeof(mem_map));
    void *mem_alloc_addr = simple_allocator_alloc(data_alloc, mem_alloc_size);
    void *rb_alloc_addr = simple_allocator_alloc(data_alloc, rb_alloc_size);
    simple_allocator *mem_alloc = simple_allocator_init(mem_alloc_addr, mem_alloc_size);
    simple_allocator *rb_alloc = simple_allocator_init(rb_alloc_addr, rb_alloc_size);
    uint8_t map_res = memory_map_parse(bootInfo, rb_alloc, mem_alloc, map);
    simple_allocator_free(data_alloc, rb_alloc_size);
    if (map_res)
    {
        for (uint64_t i = 0;i < map->length; i++)
        {
            uint64_t start = map->map[i].start;
            uint64_t end = map->map[i].end; 
            uint64_t size = end - start;
            if (end >= bootInfo->framebuffer.base && start <= bootInfo->framebuffer.base) {
                size--;
            }
        }
    } 
    else 
        kernel_loop();
    uint64_t pg_alloc_size = sizeof(simple_allocator) + bootInfo->memorymap.size;
    void *pg_alloc_addr = simple_allocator_alloc(data_alloc, pg_alloc_size);
    simple_allocator *pg_alloc = simple_allocator_init(pg_alloc_addr, pg_alloc_size);
    if (!pg_alloc)
        kernel_loop();
    page_allocator *page_alloc = page_allocator_init(map, pg_alloc);
    if (!page_alloc)
        kernel_loop();
    kernel_loop();
    return 0;
}