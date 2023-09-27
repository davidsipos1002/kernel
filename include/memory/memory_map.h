#ifndef MEMORY_MEMORY_MAP_H_INCL
#define MEMORY_MEMORY_MAP_H_INCL

#include <boot/bootinfo.h>
#include <stdint.h>
#include <memory/simple_allocator.h>

typedef struct
{
    uint64_t start;
    uint64_t end;
} mem_region;

uint64_t memmory_map_parse(BootInfo *info, simple_allocator *rb_alloc, simple_allocator *mem_alloc);

#endif