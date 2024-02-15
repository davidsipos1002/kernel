#ifndef MEMORY_PAGE_ALLOCATOR_H_INCL
#define MEMORY_PAGE_ALLOCATOR_H_INCL

#include <stdint.h>

#include <algorithm/double_linked_list.h>
#include <memory/buddy_allocator.h>
#include <memory/memory_map.h>
#include <memory/simple_allocator.h>

#define PAGE_ALLOCATOR_MIN_REGION 128

typedef struct
{
    double_linked_list_node link;
    buddy_allocator *allocator; 
} page_allocator_buddy;

typedef struct
{
    double_linked_list_node link;
    uint64_t start;
    uint64_t length;
} page_allocator_region; 

typedef struct
{
    double_linked_list buddies;
    double_linked_list regions;    
} page_allocator;

page_allocator *page_allocator_init(mem_map *map, simple_allocator *alloc); 
void *page_allocator_alloc(page_allocator *allocator, buddy_page_frame *frame, uint8_t size);
void page_allocator_free(page_allocator *allocator, buddy_page_frame *frame);

#endif