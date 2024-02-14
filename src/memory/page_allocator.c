#include <memory/page_allocator.h>

#include <algorithm/quicksort.h>
#include <gcc/utils.h>
#include <paging/paging.h>
#include <paging/state.h>

typedef struct
{
    uint64_t start;
    uint64_t length;
} free_reg;
typedef struct 
{
    uint64_t count;
    uint64_t pages;
    free_reg *regs;
    simple_allocator *alloc;
} free_regs;

static inline void ALWAYS_INLINE add_to_free_reg(free_regs *regs, uint64_t start, uint64_t length)
{
    free_reg *new_reg = simple_allocator_alloc(regs->alloc, sizeof(free_reg));
    new_reg->start = start;
    new_reg->length = length;
    regs->count++;
    regs->pages += length;
}

static inline void ALWAYS_INLINE reserve_region(mem_region *reg, uint64_t length, free_regs *regs)
{
    uint64_t temp = reg->start;
    reg->start = reg->end;
    reg->end = temp;
    add_to_free_reg(regs, reg->start, length);
}

static uint64_t shrink_region(mem_region *reg, uint64_t length, uint64_t count, free_regs *regs)
{
    if (count > length)
        return 0;
    uint64_t rem = length - count; 
    if (rem < BUDDY_ALLOCATOR_MIN_ORDER)
    {
        reserve_region(reg, length, regs);
        return length;
    }
    add_to_free_reg(regs, reg->start, count);
    reg->start += count * PAGING_PAGE_SIZE;
    return count;
}

static void region_initial_process(mem_region *region, uint64_t *buddy_total, free_regs *regs)
{
    uint64_t length = ((region->end - region->start) >> PAGING_PAGE_SIZE_EXP) + 1;
    if (length < PAGE_ALLOCATOR_MIN_REGION)
    {
        uint64_t temp = region->start;
        region->start = region->end;
        region->end = temp;
        add_to_free_reg(regs, region->start, length);
        return;
    }
    uint64_t r = length % (1 << BUDDY_ALLOCATOR_MIN_ORDER);
    if (r)
    {
        uint64_t freed = shrink_region(region, length, r, regs); 
        if (freed == length)
            return;
    }
    *buddy_total += buddy_allocator_get_size(length);
}

static uint8_t steal_pages(mem_map *map, free_regs *regs, uint64_t count)
{
    count += BUDDY_ALLOCATOR_MIN_ORDER - (count % BUDDY_ALLOCATOR_MIN_ORDER);
    for (uint64_t i = 0; i < map->length && count; i++)
    {
        mem_region *region = &(map->map[i]);
        if (region->end > region->start)
        {
            uint64_t length = ((region->end - region->start) >> PAGING_PAGE_SIZE_EXP) + 1;
            if (length <= count)
            {
                reserve_region(region, length, regs);
                count -= length;
            } 
            else
            {
                uint64_t stolen = shrink_region(region, length, count, regs);
                if (stolen >= count)
                    return 1;
                count -= stolen; 
            }
        }
    }
    return (count == 0);
}

static uint8_t reg_compar(const void *a, const void *b)
{
    free_reg *ap = (free_reg *) a;
    free_reg *bp = (free_reg *) b;
    if (ap->start <= bp->start)
        return 0;
    return 2;
}

static uint64_t get_free_pages(free_regs *regs, uint64_t *ndx, uint64_t count, uint64_t *got)
{
    if (*ndx >= regs->count)
        return 0; 
    *got = count <= regs->regs[*ndx].length ? count : regs->regs[*ndx].length;
    uint64_t addr = regs->regs[*ndx].start;
    regs->regs[*ndx].start += *got * PAGING_PAGE_SIZE; 
    regs->regs[*ndx].length -= *got;
    if (!regs->regs[*ndx].length)
        *ndx += 1;
    return addr;
}

static inline void ALWAYS_INLINE prepare_page_tables(uint64_t pd_count, uint64_t pt_count, free_regs *regs, uint64_t *free_ndx)
{
    uint64_t got = 0;
    if (pd_count >= 1)
    {
        uint64_t pdpt_ndx = 1;
        while(pd_count)
        {
            uint64_t addr = get_free_pages(regs, free_ndx, 1, &got);
            scratchpad_add_page_directory(0, pdpt_ndx, addr);
            pd_count--;
            pdpt_ndx++;
        }
    } 
    
    if (pt_count >= 1)
    {
        uint64_t pdpt_ndx = 1;
        uint64_t pd_ndx = 0;
        while (pt_count)
        {
            uint64_t addr = get_free_pages(regs, free_ndx, 1, &got);
            scratchpad_add_page_table(0, pdpt_ndx, pd_ndx, addr);
            pd_ndx++;
            if (pd_ndx >= PAGING_PAGE_TABLE_LENGTH)
            {
                pd_ndx = 0;
                pdpt_ndx++;
            }
            pt_count--;
        }
    }
}

static uint64_t map_buddy_area(uint64_t vaddr, uint64_t buddy_size, free_regs *regs, uint64_t *free_ndx)
{
    uint64_t got = 0;
    uint64_t addr = 0;
    while(buddy_size)
    {
        addr = get_free_pages(regs, free_ndx, buddy_size, &got);
        scratchpad_memory_map(vaddr, addr, got);
        vaddr += got * PAGING_PAGE_SIZE;
        buddy_size -= got;
    }
    return vaddr;
}

static uint64_t init_buddies(mem_map *map, simple_allocator *alloc, free_regs *regs, uint64_t *free_ndx, buddy_allocator ***buddies)
{
    uint64_t vaddr = paging_get_virtual_address(0, 1, 0, 0);
    uint64_t buddy_ndx = 0;
    for (uint64_t i = 0; i < map->length; i++)
        if (map->map[i].start < map->map[i].end)
            buddy_ndx++;
    buddy_allocator **allocs = simple_allocator_alloc(alloc, buddy_ndx * sizeof(buddy_allocator *));
    buddy_ndx = 0;
    for (uint64_t i = 0; i < map->length; i++)
    {
        mem_region *reg = &(map->map[i]);
        if (reg->start < reg->end)
        {
            uint64_t length = ((reg->end - reg->start) >> PAGING_PAGE_SIZE_EXP) + 1;
            uint64_t buddy_size = buddy_allocator_get_size(length);
            uint64_t next_vaddr = map_buddy_area(vaddr, buddy_size, regs, free_ndx);
            allocs[buddy_ndx++] = buddy_allocator_init((void *) vaddr, reg->start, length);
            vaddr = next_vaddr;
        }
    }
    *buddies = allocs;
    return vaddr;
}

page_allocator *page_allocator_init(mem_map *map, simple_allocator *alloc)
{
    uint64_t buddy_total = 0; 
    free_regs regs;
    regs.count = 0;
    regs.pages = 0;
    regs.regs = (free_reg *) alloc->addr;
    regs.alloc = alloc;

    for (uint64_t i = 0; i < map->length; i++)
        region_initial_process(map->map + i, &buddy_total, &regs);
    uint64_t pd_count = buddy_total / (PAGING_TABLE_LENGTH * PAGING_TABLE_LENGTH) + 
        (buddy_total % (PAGING_TABLE_LENGTH * PAGING_TABLE_LENGTH) != 0);
    uint64_t pt_count = buddy_total / PAGING_TABLE_LENGTH + (buddy_total % PAGING_TABLE_LENGTH != 0);
    uint64_t total = buddy_total + pd_count + pt_count;
    total += BUDDY_ALLOCATOR_MIN_ORDER - (total % BUDDY_ALLOCATOR_MIN_ORDER);

    if (total > regs.pages)
        if (!steal_pages(map, &regs, total - regs.pages))
            return 0;
    
    quicksort(regs.regs, regs.count, sizeof(free_reg), reg_compar);

    uint64_t free_ndx = 0;
    prepare_page_tables(pd_count, pt_count, &regs, &free_ndx);

    buddy_allocator **buddies;
    uint64_t addr = init_buddies(map, alloc, &regs, &free_ndx, &buddies);

    return (page_allocator *) 1;
} 