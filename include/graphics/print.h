#ifndef GRAPHICS_PRINT_H_INCL
#define GRAPHICS_PRINT_H_INCL

#include <stdint.h>

#include <graphics/glyph.h>

void graphics_print_string(graphics_glyph_description *desc, char *s, uint32_t row, uint32_t column, graphics_glyph_color *color);

#endif