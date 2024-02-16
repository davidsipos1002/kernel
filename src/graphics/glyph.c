#include <graphics/glyph.h>

void graphics_glyph_display(graphics_glyph_description *desc, char c, uint32_t x, uint32_t y, graphics_glyph_color *color)
{
    uint8_t row_size = desc->width / 8;
    if (desc->width % 8)
        row_size += 8 - desc->width % 8;
    uint64_t offset = ((uint8_t) c) * row_size * desc->height;
    uint8_t *glyph_addr = (uint8_t *) (desc->glyph_vaddr + 2 + offset);
    uint32_t curr_y = y;
    for (uint8_t i = 0; i < desc->height; i++)
    {
        for (int8_t j = desc->width - 1; j >= 0; j--)
        {
            if (glyph_addr[j / 8] & (1 << (j % 8)))
                graphics_framebuffer_set(&desc->framebuffer, x, curr_y, color->fg_red, color->fg_green, color->fg_blue);
            else
                graphics_framebuffer_set(&desc->framebuffer, x, curr_y, color->bg_red, color->bg_green, color->bg_blue);
            curr_y++;
        }
        x++;
        curr_y = y;
        glyph_addr += row_size;
    }
}