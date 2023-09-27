#include <memory/memory_map.h>

#include <algorithm/rb_tree.h>
#include <gcc/utils.h>
#include <memory/efi_memory.h>
#include <paging/paging.h>

static inline uint8_t ALWAYS_INLINE is_it_usable(uint32_t type)
{
    return type == EfiBootServicesCode || type == EfiBootServicesData || type == EfiConventionalMemory ||
        type == EfiLoaderCode || type == EfiLoaderData || type == EfiPersistentMemory;
}

int8_t desc_comp(uint64_t a, uint64_t b)
{
    mem_region *mem_a = (mem_region *) a;
    mem_region *mem_b = (mem_region *) b;
    return rb_simple_comparator(mem_a->end, mem_b->end);
}

uint64_t memmory_map_parse(BootInfo *info, simple_allocator *rb_alloc, simple_allocator *mem_alloc)
{
    uint64_t base = 0;
    uint64_t size = info->memorymap.size;
    uint64_t desc_size = info->memorymap.descriptor_size;
    uint64_t count = size / desc_size;
    uint64_t ret = 0;

    scratchpad_memory_map(info->memorymap.map, get_page_count(size));

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
            if (found == t->nil)
            {
                rb_node *node = simple_allocator_alloc(rb_alloc, sizeof(rb_node));
                mem_region *mem = simple_allocator_alloc(mem_alloc, sizeof(mem_region));
                if (node && mem)
                {
                    mem->start = desc->PhysicalStart;
                    mem->end = desc->PhysicalStart + desc->NumberOfPages * PAGING_PAGE_SIZE;
                    node->_key = (uint64_t) mem;
                    rb_insert(t, node);
                    ret++;
                } 
                else
                    return UINT64_MAX;
            }
            else
            {
                rb_delete(t, found);
                mem_region *curr = (mem_region *) found->_key;
                curr->end = desc->PhysicalStart + desc->NumberOfPages * PAGING_PAGE_SIZE;
                rb_insert(t, found);
            }
        }
        base += desc_size; 
    }
    return ret;
}