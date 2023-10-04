#ifndef MEMORY_BUDDY_ALLOCATOR_H_INCL
#define MEMORY_BUDDY_ALLOCATOR_H_INCL

#include <stdint.h>
#include <algorithm/linked_list.h>
#include <gcc/utils.h>
#include <memory/memory_map.h> 

#define BUDDY_ALLOCATOR_MAXORDER 10

typedef struct
{
    linked_list *list;
    uint8_t *map;
} buddy_list;

typedef struct
{
    buddy_list lists[BUDDY_ALLOCATOR_MAXORDER];
} buddy_allocator;

uint64_t buddy_allocator_get_size(uint64_t page_count);
void buddy_allocator_init(void *init_addr, uint64_t page_count);

#endif