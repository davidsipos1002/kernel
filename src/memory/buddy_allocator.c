#include <memory/buddy_allocator.h>

#include <memory/manipulate.h>
#include <paging/paging.h>

uint64_t get_list_nodes_size(uint64_t page_count, uint8_t order)
{
    return (page_count >> order) * sizeof(linked_list_node);
}

uint64_t get_map_size(uint64_t page_count, uint8_t order)
{
    return (page_count >> (order + 1)) >> 3;
}

uint64_t buddy_allocator_get_size(uint64_t page_count)
{
    uint64_t list_sizes = 0;
    uint64_t list_nodes_sizes = 0;
    uint64_t map_sizes = 0;
    for (uint64_t i = 0; i < BUDDY_ALLOCATOR_MAXORDER; i++)
    {
        list_sizes += sizeof(linked_list);
        list_nodes_sizes += get_list_nodes_size(page_count, i);
        map_sizes += get_map_size(page_count, i);
    }
    return sizeof(buddy_allocator) + list_sizes + list_nodes_sizes + map_sizes;
}

void buddy_allocator_init(void *init_addr, uint64_t page_count)
{
    uint64_t remaining_pages = page_count;
    uint64_t curr_addr = (uint64_t) init_addr;
    buddy_allocator *allocator = (buddy_allocator *) curr_addr;
    curr_addr += sizeof(buddy_allocator);
    
    for (uint32_t i = 0; i < BUDDY_ALLOCATOR_MAXORDER; i++)
    {
        linked_list *curr_list = (linked_list *) curr_addr;
        linked_list_init(curr_list);
        curr_addr += sizeof(linked_list);
        uint64_t list_size = get_list_nodes_size(page_count, i);
        memset((void *) curr_addr, 0, list_size);
        curr_addr += list_size;
        uint8_t *curr_map = (uint8_t *) curr_addr;
        uint64_t map_size = get_map_size(page_count, i); 
        memset((void *) curr_map, 0, map_size);
        curr_addr += map_size;
        allocator->lists[i].list = curr_list;
        allocator->lists[i].map = curr_map;
        if (remaining_pages)
        {
            uint64_t list_count = remaining_pages >> i;
            linked_list_node *node_begin = (linked_list_node *) (curr_list + 1);
            for (uint64_t j = 0; j < list_count; j++)
            {
                node_begin[j].key = j * (1 << i);
                linked_list_insert_rear(curr_list, &node_begin[j]);
            }
            remaining_pages = remaining_pages & ((1 << i) - 1);
        }
    }
}
