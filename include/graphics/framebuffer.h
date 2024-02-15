#ifndef GRAPHICS_FRAMEBUFFER_H_INCL
#define GRAPHICS_FRAMEBUFFER_H_INCL

#include <stdint.h>

#include <boot/bootinfo.h>
#include <memory/page_allocator.h>

typedef struct
{
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t pixel_size;
    uint64_t base; 
    uint64_t vaddr;
} graphics_framebuffer;

void graphics_framebuffer_init(BootInfo *bootInfo, graphics_framebuffer *framebuffer, page_allocator *alloc);
void graphics_framebuffer_set(graphics_framebuffer *framebuffer, uint32_t x, uint32_t y, uint8_t red, uint8_t green, uint8_t blue);

#endif