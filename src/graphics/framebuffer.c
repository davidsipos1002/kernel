#include <graphics/framebuffer.h>

#include <gcc/utils.h>
#include <memory/page_allocator.h>
#include <paging/paging.h>

static uint64_t alloc_page_tables(graphics_framebuffer *framebuffer, page_allocator *alloc, uint64_t *page_count, uint64_t *total, uint64_t *pdp_count, uint64_t *pd_count, uint64_t *pt_count)
{
    uint64_t framebuffer_size = framebuffer->height * framebuffer->pitch * framebuffer->pixel_size;
    *page_count = framebuffer_size / PAGING_PAGE_SIZE + (framebuffer_size % PAGING_PAGE_SIZE != 0);   
    *pdp_count = *page_count / (PAGING_TABLE_LENGTH * PAGING_TABLE_LENGTH * PAGING_PAGE_TABLE_LENGTH) + 
        (*page_count % (PAGING_TABLE_LENGTH * PAGING_TABLE_LENGTH * PAGING_PAGE_TABLE_LENGTH) != 0);
    *pd_count = *page_count / (PAGING_TABLE_LENGTH * PAGING_TABLE_LENGTH) + 
        (*page_count % (PAGING_TABLE_LENGTH * PAGING_TABLE_LENGTH) != 0);
    *pt_count = *page_count / PAGING_TABLE_LENGTH + (*page_count % PAGING_TABLE_LENGTH != 0);
    *total = *pdp_count + *pd_count + *pt_count;

    buddy_page_frame frame;
    uint64_t order = 0;
    for (uint64_t i = BUDDY_ALLOCATOR_MIN_ORDER; i <= BUDDY_ALLOCATOR_MAX_ORDER; i++)
    {
        if (*total <= ((uint64_t) 1 << i))
        {
            order = i;
            break;
        }
    }
    return (uint64_t) page_allocator_alloc(alloc, &frame, order); 
}

static uint64_t prepare_page_tables(uint64_t addr, uint64_t pdp_count, uint64_t pd_count, uint64_t pt_count)
{
    // we will map the framebuffer to the last possible addresses
    uint64_t pml4i = 510 - pdp_count + 1;
    for (uint64_t i = 0; i < pdp_count; i++)
    {
        scratchpad_add_page_directory_pointer_table(pml4i, addr);
        addr += PAGING_PAGE_SIZE;
        pml4i++;
    }

    pml4i = 510 - pdp_count + 1;
    uint64_t pdpi = 0;
    for (uint64_t i = 0; i < pd_count; i++)
    {
        scratchpad_add_page_directory(pml4i, pdpi, addr);
        addr += PAGING_PAGE_SIZE;
        pdpi++;
        if (pdpi >= PAGING_PAGE_TABLE_LENGTH)
        {
            pdpi = 0;
            pml4i++;
        } 
    }
    
    pml4i = 510 - pdp_count + 1;
    pdpi = 0;
    uint64_t pdi = 0;
    for (uint64_t i = 0; i < pt_count; i++)
    {
        scratchpad_add_page_table(pml4i, pdpi, pdi, addr);
        addr += PAGING_PAGE_SIZE;
        pdi++;
        if (pdi >= PAGING_PAGE_TABLE_LENGTH)
        {
            pdi = 0;
            pdpi++;
            if (pdpi >= PAGING_PAGE_TABLE_LENGTH)
            {
                pdpi = 0;
                pml4i++;
            }
        }
    }
    return paging_get_virtual_address(510 - pdp_count + 1, 0, 0, 0);
}

void graphics_framebuffer_init(BootInfo *bootInfo, graphics_framebuffer *framebuffer, page_allocator *alloc)
{
    framebuffer->width = bootInfo->framebuffer.width;
    framebuffer->height = bootInfo->framebuffer.height;
    framebuffer->pitch = bootInfo->framebuffer.pitch;
    framebuffer->pixel_size = bootInfo->framebuffer.pixel_size;
    framebuffer->base = bootInfo->framebuffer.base;

    uint64_t page_count, total, pdp_count, pd_count, pt_count;
    uint64_t addr = alloc_page_tables(framebuffer, alloc, &page_count, &total, &pdp_count, &pd_count, &pt_count);
    
    uint64_t vaddr = prepare_page_tables(addr, pdp_count, pd_count, pt_count);
    scratchpad_memory_map(vaddr, framebuffer->base, page_count);
    framebuffer->vaddr = vaddr;
}

void graphics_framebuffer_set(graphics_framebuffer *framebuffer, uint32_t x, uint32_t y, uint8_t red, uint8_t green, uint8_t blue)
{
    if (x >= framebuffer->height || y >= framebuffer->width)
        return;
    uint32_t *pixel = (uint32_t *) (((uint8_t *) framebuffer->vaddr) + x * framebuffer->pitch * framebuffer->pixel_size +
        y * framebuffer->pixel_size);
    *pixel = ((uint32_t) red << 16) | ((uint32_t) green << 8) | ((uint32_t) blue);
}