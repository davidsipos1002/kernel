#include <asm/registers.h>
#include <boot/bootinfo.h>
#include <paging/format.h>
#include <paging/paging.h>
#include <memory/simple_allocator.h>
#include <memory/memory_map.h>
#include <algorithm/linked_list.h>
#include <asm/random.h>

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
        scratchpad_memory_map(0, bootInfo->framebuffer.base, 10);
        uint8_t *buffer = 0;
        for (uint64_t i = 0; i < 1920; i++)
        {
            *((uint32_t *) (buffer + bootInfo->framebuffer.pixel_size * i)) = 0xFF00;
        }
    }
    start = (uint64_t) __kernel_data_begin + sizeof(paging_state);
    linked_list list;
    simple_allocator *alloc = simple_allocator_init((void *) start, 105 * sizeof(linked_list_node));
    linked_list_init(&list);
    linked_list_node *a = simple_allocator_alloc(alloc, sizeof(linked_list_node));
    a->key = 10;
    linked_list_insert_front(&list, a);
    linked_list_node *b = simple_allocator_alloc(alloc, sizeof(linked_list_node));
    b->key = 20;
    linked_list_insert_front(&list, b);
    linked_list_node *c = simple_allocator_alloc(alloc, sizeof(linked_list_node));
    c->key = 15;
    linked_list_insert_front(&list, c);
    linked_list_node *find = linked_list_find(&list, 20);
    linked_list_delete(&list, 20);
    find = linked_list_find(&list, 20);
    find = linked_list_find(&list, 15);
    uint64_t test = 0;
    uint8_t ret = 0;
    if (random_support())
    {
        for (int i = 0; i < 5; i++) 
        {
            ret = random_rdrand64_retry(&test, RANDOM_RDRAND_RETRIES);
        }
    }
    while (1);
    return 0;
}