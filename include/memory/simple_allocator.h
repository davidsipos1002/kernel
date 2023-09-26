#ifndef MEMORY_SIMPLE_ALLOCATOR_H_INCL
#define MEMORY_SIMPLE_ALLOCATOR_H_INCL

#include <stdint.h>

typedef struct 
{
    uint64_t addr;
    uint64_t size;
} simple_allocator;

simple_allocator* simple_allocator_init(void *start_addr, uint64_t max_size);
void* simple_allocator_alloc(simple_allocator *alloc, uint64_t size);
uint8_t simple_allocator_align(simple_allocator *alloc, uint64_t align);

#endif