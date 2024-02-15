#include <memory/buddy_allocator.h>

#include <algorithm/bit_field.h>
#include <memory/manipulate.h>

static uint8_t buddy_allocator_highest_order(uint64_t x)
{
    uint64_t msk = (uint64_t) 1 << 63;
    uint8_t i = 63;
    while (msk && !(x & msk))
    {
        i--;
        msk >>= 1;
    }
    return i;
}

uint64_t buddy_allocator_get_size(uint64_t size)
{
    uint8_t highest_order = buddy_allocator_highest_order(size);
    if (size <= 1 || highest_order < BUDDY_ALLOCATOR_MIN_ORDER)
        return 0;

    uint8_t max_order = highest_order < BUDDY_ALLOCATOR_MAX_ORDER ? highest_order : BUDDY_ALLOCATOR_MAX_ORDER;
    uint64_t byte_count = sizeof(buddy_allocator) + (max_order - BUDDY_ALLOCATOR_MIN_ORDER + 1) * sizeof(void *);

    for (uint8_t i = BUDDY_ALLOCATOR_MIN_ORDER; i <= max_order; i++)
    {
        uint64_t list_count = size >> i;
        uint64_t list_size = list_count * sizeof(buddy_block);
        if (i != max_order)
        {
            uint64_t bit_count = list_count >> 1;
            uint64_t bitmap_size = (bit_count >> 3) + ((bit_count & 0x7) != 0);
            byte_count += bitmap_size;
        }
        byte_count += sizeof(buddy_list) + list_size;
    }
    uint64_t page_count = (byte_count >> PAGING_PAGE_SIZE_EXP) + ((byte_count & (PAGING_PAGE_SIZE - 1)) != 0);
    return page_count;
}

static inline buddy_block *buddy_allocator_addr_to_block(buddy_list *list, uint64_t start_addr, uint64_t addr, uint8_t size)
{
    uint64_t ndx = ((addr - start_addr) >> size) >> PAGING_PAGE_SIZE_EXP;
    return ((buddy_block *) (list + 1)) + ndx;
}

static void buddy_allocator_list_insert(buddy_list *list, buddy_block *block)
{
    if (!list->head)
    {
        block->prev = block;
        block->next = block;
        list->head = block;
        return;
    }
    block->prev = list->head->prev;
    block->next = list->head;
    list->head->prev->next = block;
    list->head->prev = block;
    list->head = block;
}

static void buddy_allocator_list_remove(buddy_list *list)
{
    if (list->head->next == list->head)
    {
        list->head = 0;
        return;
    }
    buddy_block *snd = list->head->next;
    snd->prev = list->head->prev;
    list->head->prev->next = snd;
    list->head->prev = 0;
    list->head->next = 0;
    list->head = snd;
}

static void buddy_allocator_list_remove_block(buddy_list *list, buddy_block *block)
{
    if (list->head == block)
    {
        buddy_allocator_list_remove(list);
        return;
    }
    block->prev->next = block->next;
    block->next->prev = block->prev;
    block->prev = 0;
    block->next = 0;
}

static inline void ALWAYS_INLINE buddy_allocator_init_fill(buddy_allocator *allocator)
{
    uint8_t max_order = allocator->max_order;
    uint64_t page_count = allocator->size;
    uint8_t li = max_order - BUDDY_ALLOCATOR_MIN_ORDER;
    uint64_t addr = allocator->start_addr;
    for (uint8_t i = max_order; i >= BUDDY_ALLOCATOR_MIN_ORDER && page_count; i--)
    {
        uint64_t block_size = (uint64_t) 1 << i;
        uint64_t block_byte_size = block_size << PAGING_PAGE_SIZE_EXP;
        buddy_list *list = allocator->lists[li];
        while (page_count >= block_size)
        {
            buddy_block *block_to_insert = buddy_allocator_addr_to_block(list, allocator->start_addr, addr, i);
            block_to_insert->stop = 1;
            buddy_allocator_list_insert(list, block_to_insert);
            addr += block_byte_size;
            page_count -= block_size;
        }
        li--;
    }
}

buddy_allocator *buddy_allocator_init(void *buddy_addr, uint64_t start_addr, uint64_t size)
{
    uint8_t highest_order = buddy_allocator_highest_order(size);
    if (size <= 1 || highest_order < BUDDY_ALLOCATOR_MIN_ORDER)
        return 0;

    uint64_t addr = (uint64_t) buddy_addr;
    buddy_allocator *allocator = (buddy_allocator *) addr;
    allocator->start_addr = start_addr;
    allocator->size = size;
    allocator->max_order = highest_order < BUDDY_ALLOCATOR_MAX_ORDER ? highest_order : BUDDY_ALLOCATOR_MAX_ORDER;
    addr += sizeof(buddy_allocator);
    allocator->lists = (buddy_list **) addr;
    addr += (allocator->max_order - BUDDY_ALLOCATOR_MIN_ORDER + 1) * sizeof(void*);

    uint8_t li = 0;
    for (uint8_t i = BUDDY_ALLOCATOR_MIN_ORDER; i <= allocator->max_order; i++)
    {
        buddy_list *list = (buddy_list *) addr;
        list->head = 0;
        list->bitmap = 0;
        allocator->lists[li++] = list;
        addr += sizeof(buddy_list);
        uint64_t list_count = size >> i;
        uint64_t list_size = list_count * sizeof(buddy_block);
        addr += list_size;
        if (i != allocator->max_order)
        {
            uint64_t bit_count = list_count >> 1;
            uint64_t bitmap_size = (bit_count >> 3) + ((bit_count & 0x7) != 0);
            list->bitmap = (uint8_t *) addr;
            memset(list->bitmap, 0, bitmap_size);
            addr += bitmap_size;
        }
    }
    buddy_allocator_init_fill(allocator);
    return (buddy_allocator *) buddy_addr;
}

static inline uint64_t buddy_allocator_block_to_addr(uint64_t start_addr, buddy_list *list, buddy_block *block, uint8_t size)
{
    return start_addr + (((uint64_t) (block - (buddy_block *) (list + 1)) << size) << PAGING_PAGE_SIZE_EXP);
}

static inline uint64_t buddy_allocator_get_buddy(uint64_t addr, uint8_t size)
{
    return addr ^ ((uint64_t) 1 << (size + PAGING_PAGE_SIZE_EXP));
}

static inline uint64_t buddy_allocator_get_aligned_buddy(uint64_t addr, uint8_t size)
{
    return addr & (~((uint64_t) 1 << (size + PAGING_PAGE_SIZE_EXP)));
}

static inline void ALWAYS_INLINE buddy_allocator_bitmap_flip(buddy_list *list, buddy_block *block)
{
    uint64_t bit_no = (((uint64_t) (block - (buddy_block *) (list + 1))) & (~0x1)) >> 1;
    bit_field_toggle_bit(list->bitmap, bit_no);
}

static inline uint8_t buddy_allocator_bitmap_get(buddy_list *list, buddy_block *block)
{
    uint64_t bit_no = (((uint64_t) (block - (buddy_block *) (list + 1))) & (~0x1)) >> 1;
    return bit_field_get_bit(list->bitmap, bit_no);
}

void buddy_allocator_alloc(buddy_allocator *allocator, buddy_page_frame *frame)
{
    frame->addr = 0;

    if (frame->size < BUDDY_ALLOCATOR_MIN_ORDER && frame->size > allocator->max_order)
        return;

    uint8_t frame_ndx = frame->size - BUDDY_ALLOCATOR_MIN_ORDER;
    uint8_t ndx = frame_ndx;
    uint8_t max_ndx = allocator->max_order - BUDDY_ALLOCATOR_MIN_ORDER;
    while (!allocator->lists[ndx]->head && ndx <= max_ndx)
        ndx++;

    if (ndx > max_ndx)
        return;

    uint8_t ndx_order = ndx + BUDDY_ALLOCATOR_MIN_ORDER;
    while (ndx > frame_ndx)
    {
        buddy_list *list_to_split = allocator->lists[ndx];
        buddy_block *block_to_split = list_to_split->head;
        if (ndx != max_ndx)
            buddy_allocator_bitmap_flip(list_to_split, block_to_split);
        buddy_allocator_list_remove(list_to_split);
        uint64_t addr = buddy_allocator_block_to_addr(allocator->start_addr, list_to_split, block_to_split, ndx_order);
        ndx_order--;
        ndx--;
        uint64_t buddy_addr = buddy_allocator_get_buddy(addr, ndx_order);
        buddy_list *list_to_insert = allocator->lists[ndx];
        buddy_allocator_list_insert(list_to_insert, buddy_allocator_addr_to_block(list_to_insert, allocator->start_addr, addr, ndx_order));
        buddy_allocator_list_insert(list_to_insert, buddy_allocator_addr_to_block(list_to_insert, allocator->start_addr, buddy_addr, ndx_order));
    }

    buddy_list *list = allocator->lists[ndx];
    buddy_block *block = list->head;
    buddy_allocator_list_remove(list);
    if (ndx != max_ndx)
        buddy_allocator_bitmap_flip(list, block);
    frame->allocator = allocator;
    frame->addr = buddy_allocator_block_to_addr(allocator->start_addr, list, block, ndx_order);
}

void buddy_allocator_free(buddy_allocator *allocator, buddy_page_frame *frame)
{
    uint8_t ndx = frame->size - BUDDY_ALLOCATOR_MIN_ORDER;
    uint8_t ndx_order = frame->size;
    uint64_t addr = frame->addr;
    buddy_list *list = allocator->lists[ndx];
    buddy_block *block = buddy_allocator_addr_to_block(list, allocator->start_addr, addr, ndx_order);
    buddy_allocator_list_insert(list, block);
    if (ndx_order != allocator->max_order)
        buddy_allocator_bitmap_flip(list, block);

    while (ndx_order < allocator->max_order && !block->stop && !buddy_allocator_bitmap_get(list, block))
    {
        uint64_t buddy_addr = buddy_allocator_get_buddy(addr, ndx_order);
        buddy_block *buddy = buddy_allocator_addr_to_block(list, allocator->start_addr, buddy_addr, ndx_order);
        buddy_allocator_list_remove_block(list, block);
        buddy_allocator_list_remove_block(list, buddy);
        addr = buddy_allocator_get_aligned_buddy(addr, ndx_order);
        ndx++;
        ndx_order++;
        list = allocator->lists[ndx];
        block = buddy_allocator_addr_to_block(list, allocator->start_addr, addr, ndx_order);
        buddy_allocator_list_insert(list, block);
        if (ndx_order != allocator->max_order)
            buddy_allocator_bitmap_flip(list, block);
    }

    frame->allocator = 0;
    frame->size = 0;
    frame->addr = 0;
}
