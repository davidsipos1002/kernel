#include <memory/simple_allocator.h>

simple_allocator* simple_allocator_init(void *start_addr, uint64_t max_size)
{
    simple_allocator *alloc = (simple_allocator *) start_addr;
    alloc->addr = (uint64_t) start_addr + sizeof(simple_allocator);
    alloc->size = max_size - sizeof(simple_allocator); 
    return (simple_allocator *) alloc; 
}

void* simple_allocator_alloc(simple_allocator *alloc, uint64_t size)
{
    if (size > alloc->size)
        return (void *) 0; 
    uint64_t allocated = alloc->addr;
    alloc->addr += size;
    alloc->size -= size;
    return (void *) allocated;
}

uint8_t simple_allocator_align(simple_allocator *alloc, uint64_t align)
{
    uint64_t skip = align - (alloc->addr % align);
    if (skip > alloc->size)
        return 0;
    alloc->addr += skip;
    alloc->size -= skip;
    return 1; 
}