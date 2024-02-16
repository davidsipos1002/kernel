#ifndef GRAPHICS_GLYPH_H_INCL
#define GRAPHICS_GLYPH_H_INCL

#include <stdint.h>

#include <graphics/framebuffer.h>

typedef struct
{
    uint64_t glyph_vaddr;
    uint8_t width;
    uint8_t height;
    graphics_framebuffer framebuffer;
} graphics_glyph_description;

typedef struct
{
    uint8_t bg_red;
    uint8_t bg_green;
    uint8_t bg_blue;
    uint8_t fg_red;
    uint8_t fg_green;
    uint8_t fg_blue;
} graphics_glyph_color;

void graphics_glyph_display(graphics_glyph_description *desc, char c, uint32_t x, uint32_t y, graphics_glyph_color *color);

#endif