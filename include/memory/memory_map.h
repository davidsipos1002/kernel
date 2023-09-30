#ifndef MEMORY_MEMORY_MAP_H_INCL
#define MEMORY_MEMORY_MAP_H_INCL

#include <boot/bootinfo.h>
#include <stdint.h>
#include <memory/simple_allocator.h>
#include <algorithm/rb_tree.h>

typedef struct
{
    uint64_t start;
    uint64_t end;
} mem_region;

typedef struct 
{
    uint64_t length;
    uint64_t n_pages;
    mem_region *map;
    uint64_t *addr_map;
} mem_map;

uint8_t memory_map_parse(BootInfo *info, simple_allocator *rb_alloc, simple_allocator *mem_alloc, mem_map *map);
uint8_t memory_map_construct_map(mem_map *memory_map);

#endif