#ifndef MEMORY_BUDDY_ALLOCATOR_H_INCL
#define MEMORY_BUDDY_ALLOCATOR_H_INCL

#include <stdint.h>

#include <paging/state.h>

#define BUDDY_ALLOCATOR_MIN_ORDER 2
#define BUDDY_ALLOCATOR_MAX_ORDER 8

typedef struct buddy_block
{
    struct buddy_block *prev;
    struct buddy_block *next;
} buddy_block;

typedef struct
{
    buddy_block *head;
    uint8_t *bitmap;
} buddy_list;

typedef struct
{
    void *start_addr;
    uint8_t size;
    uint8_t max_order;
    buddy_list **lists;
} buddy_allocator;

typedef struct
{
    buddy_allocator *allocator;
    uint8_t size;
    void *addr;
} buddy_page_frame;

uint64_t buddy_allocator_get_size(uint8_t size);
buddy_allocator *buddy_allocator_init(void *buddy_addr, void *start_addr, uint8_t size);
void buddy_allocator_alloc(buddy_allocator *allocator, buddy_page_frame *frame);
void buddy_allocator_free(buddy_allocator *allocator, buddy_page_frame *frame);

#endif