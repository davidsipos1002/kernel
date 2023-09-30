#include <asm/registers.h>
#include <boot/bootinfo.h>
#include <paging/format.h>
#include <paging/paging.h>
#include <memory/simple_allocator.h>
#include <memory/memory_map.h>

extern char __kernel_data_begin[];
extern char __kernel_data_end[];

int kernel_main(BootInfo *bootInfo) 
{
    paging_state* state = paging_init(get_cr3(), (void *) __kernel_data_begin);
    simple_allocator *rb_alloc, *mem_alloc;
    uint64_t start = (uint64_t) __kernel_data_begin + sizeof(paging_state);
    rb_alloc = simple_allocator_init((void *) start, bootInfo->memorymap.size);
    start += bootInfo->memorymap.size;
    mem_alloc = simple_allocator_init((void *) start, bootInfo->memorymap.size / 2);
    start += sizeof(simple_allocator); 
    mem_map map;
    if(memory_map_parse(bootInfo, rb_alloc, mem_alloc, &map) && memory_map_construct_map(&map))
    {
        scratchpad_memory_map(bootInfo->framebuffer.base, 0, 0, 10);
        uint8_t *buffer = 0;
        for (uint64_t i = 0; i < 1920; i++)
        {
            *((uint32_t *) (buffer + bootInfo->framebuffer.pixel_size * i)) = 0xFF00;
        }
    }
    while (1);
    return 0;
}