#include <memory/page_allocator.h>

#include <algorithm/quicksort.h>
#include <gcc/utils.h>
#include <memory/manipulate.h>
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
    add_to_free_reg(regs, reg->start, length);
    uint64_t temp = reg->start;
    reg->start = reg->end;
    reg->end = temp;
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
        reserve_region(region, length, regs);
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
    regs->pages -= *got;
    return addr;
}

static inline void ALWAYS_INLINE prepare_page_tables(uint64_t pd_count, uint64_t pt_count, free_regs *regs, uint64_t *free_ndx)
{
    uint64_t got = 0;
    uint64_t pdpt_ndx = 1;
    uint64_t pd_ndx = 0;
    uint64_t addr = 0;
    while(pd_count)
    {
        addr = get_free_pages(regs, free_ndx, 1, &got);
        scratchpad_add_page_directory(0, pdpt_ndx, addr);
        pd_count--;
        pdpt_ndx++;
    } 
    
    pdpt_ndx = 1;
    while (pt_count)
    {
        addr = get_free_pages(regs, free_ndx, 1, &got);
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

static uint64_t init_buddies(mem_map *map, simple_allocator *alloc, free_regs *regs, uint64_t *free_ndx, buddy_allocator ***buddies, uint64_t *buddy_count)
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
    *buddy_count = buddy_ndx;
    return vaddr;
}

static inline void ALWAYS_INLINE prepare_page_alloc_struct_area(uint64_t struct_size, uint64_t vaddr, free_regs *regs, uint64_t *free_ndx, buddy_allocator *alloc, uint64_t last_pd)
{
    struct_size = struct_size / PAGING_PAGE_SIZE + (struct_size % PAGING_PAGE_SIZE != 0);
    uint64_t pt_count = struct_size / PAGING_TABLE_LENGTH 
        + (struct_size % PAGING_TABLE_LENGTH != 0);
    struct_size = struct_size + pt_count + 1;
    
    if (struct_size > regs->pages)
    {
        uint64_t sz = struct_size - regs->pages;
        buddy_page_frame frame;
        for (uint64_t i = BUDDY_ALLOCATOR_MIN_ORDER; i <= BUDDY_ALLOCATOR_MAX_ORDER; i++)
        {
            if (sz <= ((uint64_t) 1 << i))
            {
                frame.size = i;
                break;
            }
        }
        buddy_allocator_alloc(alloc, &frame);
        add_to_free_reg(regs, frame.addr, 1 << frame.size); 
    }
    
    uint64_t got = 0;
    uint64_t paddr = get_free_pages(regs, free_ndx, 1, &got);
    scratchpad_add_page_directory(0, last_pd + 1, paddr);
    uint64_t pdpti = last_pd;
    uint64_t pdi = pt_count % PAGING_PAGE_TABLE_LENGTH; 
    if (!pdi)
        pdpti++;
    struct_size -= pt_count + 1;
    while(pt_count)
    {
        paddr = get_free_pages(regs, free_ndx, 1, &got);
        scratchpad_add_page_table(0, pdpti, pdi, paddr);
        pdi++;
        if (pdi >= PAGING_PAGE_TABLE_LENGTH)
        {
            pdi = 0;
            pdpti++;
        }
        pt_count--;
    }
    map_buddy_area(vaddr, struct_size, regs, free_ndx);
    memset((void *) vaddr, 0, struct_size * 4096);
}

static inline void ALWAYS_INLINE page_alloc_struct_fill(uint64_t vaddr, buddy_allocator **buddies, uint64_t buddy_count, free_regs *regs)
{
    page_allocator *page_alloc = (page_allocator *) vaddr;
    double_linked_list_init(&page_alloc->buddies);
    double_linked_list_init(&page_alloc->regions); 
    vaddr += sizeof(page_allocator); 
    
    for (uint64_t i = 0;i < buddy_count; i++)
    {
        page_allocator_buddy *buddy = (page_allocator_buddy *) vaddr;
        buddy->allocator = buddies[i];
        double_linked_list_insert_back(&page_alloc->buddies, (double_linked_list_node *) buddy);
        vaddr += sizeof(page_allocator_buddy);
    }
    
    for (uint64_t i = 0;i < regs->count; i++)
    {
        free_reg *r = &regs->regs[i];
        if (r->length)
        {
            page_allocator_region *region = (page_allocator_region *) vaddr;
            region->start = r->start;
            region->length = r->length;
            double_linked_list_insert_back(&page_alloc->regions, (double_linked_list_node *) region);
            vaddr += sizeof(page_allocator_region);
        }
    }
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
    uint64_t buddy_count = 0;
    uint64_t addr = init_buddies(map, alloc, &regs, &free_ndx, &buddies, &buddy_count);
    
    uint64_t free_reg_count = 0;
    for (uint64_t i = 0;i < regs.count; i++)
        if (regs.regs[i].length)
            free_reg_count++;
    uint64_t struct_size = sizeof(page_allocator) + 
        sizeof(page_allocator_buddy) * buddy_count + sizeof(page_allocator_region) * (free_reg_count + 1);

    prepare_page_alloc_struct_area(struct_size, addr, &regs, &free_ndx, buddies[0], pd_count);

    page_alloc_struct_fill(addr, buddies, buddy_count, &regs);

    return (page_allocator *) addr;
} 

void *page_allocator_alloc(page_allocator *allocator, buddy_page_frame *frame, uint8_t size)
{
    frame->size = size;
    double_linked_list_node *curr = allocator->buddies.head;
    do
    {
        page_allocator_buddy *buddy = (page_allocator_buddy *) curr;
        buddy_allocator_alloc(buddy->allocator, frame);
        if (frame->addr)
            break;
        curr = curr->next;
    } while (curr != allocator->buddies.head);
    return (void *) frame->addr; 
}

void page_allocator_free(page_allocator *allocator, buddy_page_frame *frame)
{
    buddy_allocator_free(frame->allocator, frame); 
}