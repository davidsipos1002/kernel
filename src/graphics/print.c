#include <graphics/print.h>

void graphics_print_string(graphics_glyph_description *desc, char *s, uint32_t row, uint32_t column, graphics_glyph_color *color)
{
    uint64_t x = desc->height * row; 
    uint64_t y = desc->width * column;
    while (*s)
    {
        graphics_glyph_display(desc, *s, x, y, color);
        y += desc->width;
        s++; 
    }    
}