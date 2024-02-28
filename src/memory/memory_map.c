#include <memory/memory_map.h>

#include <algorithm/rb_tree.h>
#include <efi/memory.h>
#include <gcc/utils.h>
#include <memory/manipulate.h>
#include <paging/paging.h>
#include <stdbool.h>

static inline uint8_t ALWAYS_INLINE is_it_usable(uint32_t type)
{
    return type == EfiBootServicesCode || type == EfiBootServicesData || type == EfiConventionalMemory ||
        type == EfiLoaderCode || type == EfiLoaderData;
}

int8_t desc_comp(uint64_t a, uint64_t b)
{
    mem_region *mem_a = (mem_region *) a;
    mem_region *mem_b = (mem_region *) b;
    return rb_simple_comparator(mem_a->end, mem_b->end);
}

uint8_t memory_map_parse(BootInfo *info, simple_allocator *rb_alloc, simple_allocator *mem_alloc, mem_map *map)
{
    uint64_t base = 0;
    uint64_t size = info->memorymap.size;
    uint64_t desc_size = info->memorymap.descriptor_size;
    uint64_t count = size / desc_size;
    uint64_t length = 0;
    uint64_t n_pages = 0;
    map->addr_map = (uint64_t *) 0;
    map->map = (mem_region *) mem_alloc->addr; 

    scratchpad_memory_map(0, info->memorymap.map, get_page_count(size));

    simple_allocator_align(rb_alloc, sizeof(8));
    rb_tree *t = simple_allocator_alloc(rb_alloc, sizeof(rb_tree));
    simple_allocator_align(rb_alloc, sizeof(8));
    rb_init_tree(t, desc_comp); 

    mem_region scratch;

    for(uint64_t i = 0; i < count; i++)
    {
        EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR *) base;
        
        if (is_it_usable(desc->Type))
        {
            if (!desc->PhysicalStart)
                scratch.end = 0;
            else
                scratch.end = desc->PhysicalStart - PAGING_PAGE_SIZE;
            rb_node *found = rb_find(t, (uint64_t) &scratch);
            n_pages += desc->NumberOfPages;
            if (found == t->nil)
            {
                rb_node *node = simple_allocator_alloc(rb_alloc, sizeof(rb_node));
                mem_region *mem = simple_allocator_alloc(mem_alloc, sizeof(mem_region));
                if (node && mem)
                {
                    mem->start = desc->PhysicalStart;
                    mem->end = desc->PhysicalStart + (desc->NumberOfPages - 1) * PAGING_PAGE_SIZE;
                    node->_key = (uint64_t) mem;
                    rb_insert(t, node);
                    length++;
                } 
                else
                    return 0;
            }
            else
            {
                rb_delete(t, found);
                mem_region *curr = (mem_region *) found->_key;
                curr->end = desc->PhysicalStart + (desc->NumberOfPages - 1) * PAGING_PAGE_SIZE;
                rb_insert(t, found);
            }
        }
        base += desc_size; 
    }
    mem_region *freed_efi_map = simple_allocator_alloc(mem_alloc, sizeof(mem_region));
    if (freed_efi_map)
    {
        freed_efi_map->start = info->memorymap.map;
        uint64_t freed_efi_map_pg_count = get_page_count(info->memorymap.size);
        freed_efi_map->end = freed_efi_map->start + (freed_efi_map_pg_count - 1) * PAGING_PAGE_SIZE; 
        map->length = length + 1;
        map->n_pages = n_pages + freed_efi_map_pg_count;
        return 1;
    }
    return 0;
}

uint8_t memory_map_construct_map(mem_map *memory_map)
{
    uint64_t page_count = get_page_count(memory_map->n_pages * sizeof(uint64_t));
    uint64_t page_table_count = 0;
    uint64_t start_addr = UINT64_MAX;
    if (page_count > PAGING_PAGE_TABLE_LENGTH)
    {
        uint64_t remaining_page_count = page_count - PAGING_PAGE_TABLE_LENGTH;
        page_table_count = remaining_page_count / PAGING_PAGE_TABLE_LENGTH + (remaining_page_count % PAGING_PAGE_TABLE_LENGTH != 0);
    }
    for (uint64_t i = 0; i < memory_map->length && (page_table_count || page_count); i++)
    {
        uint64_t reg_length = (memory_map->map[i].end - memory_map->map[i].start) / PAGING_PAGE_SIZE + 1;
        if (page_table_count && reg_length > page_table_count)
        { 
            uint64_t pt_addr = memory_map->map[i].start;
            for (uint64_t j = 0; j < page_table_count; j++)
            {
                scratchpad_add_page_table(0, 0, j + 1, pt_addr);
                pt_addr += PAGING_PAGE_SIZE;
            }
            memory_map->map[i].start = pt_addr;
            reg_length -= page_table_count;
            page_table_count = 0;
        }
        if (page_count && reg_length > page_count)
        {
            uint64_t pg_addr = memory_map->map[i].start;
            start_addr = pg_addr; 
            for (uint64_t j = 0; j < page_count;j++)
            {
                scratchpad_memory_map(j * PAGING_PAGE_SIZE, pg_addr, 1);
                pg_addr += PAGING_PAGE_SIZE;
            }
            memory_map->map[i].start = pg_addr;
            reg_length -= page_count;
            page_count = 0;
        }
    }

    if (page_count || page_table_count)
        return 0;

    memory_map->addr_map = (uint64_t *) start_addr;
    uint64_t* mem = (uint64_t *) 0;
    uint64_t mem_index = 0;
    for (uint64_t i = 0; i < memory_map->length; i++)
        for (uint64_t addr = memory_map->map[i].start; addr <= memory_map->map[i].end; addr += PAGING_PAGE_SIZE)
            mem[mem_index++] = addr;
    return 1;
}