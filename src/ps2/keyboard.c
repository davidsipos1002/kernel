#include <ps2/keyboard.h>

#include <algorithm/bit_field.h>
#include <asm/io.h>
#include <gcc/interrupt.h>
#include <gcc/utils.h>
#include <graphics/print.h>
#include <interrupt/idt.h>
#include <interrupt/pic.h>
#include <memory/manipulate.h>
#include <ps2/controller.h>

#define KEYMAP_ADDR 0x18FFD000
typedef struct
{
    uint8_t normal[256];
    uint8_t emap[256];
    uint16_t ascii[256];
} PACKED_STRUCT key_map; 

static key_map *keymap = (key_map *) KEYMAP_ADDR;
static uint8_t keystat[32];

void INTERRUPT ps2_keyboard_irq(no_priv_change_frame *stack_frame)
{
    uint8_t data = io_in(PS2_DATA);
    uint8_t e, released;
    e = released = 0;
    if (data == 0xE0) 
    {
        e = 1;
        data = io_in(PS2_DATA);
    } 
    if (data == 0xF0)
    {
        released = 1;
        data = io_in(PS2_DATA);
    }
    uint8_t pos = e ? keymap->emap[data] : keymap->normal[data];
    if (released)
        bit_field_unset_bit(keystat, pos);
    else
        bit_field_set_bit(keystat, pos);
    char ascii = keymap->ascii[pos] & 0xFF;
    pic_eoi(1);
}

void ps2_keyboard_init(void *idt, uint8_t irq)
{
    memset(keystat, 0, 32);
    idt_register_handler(idt, irq, (idt_handler) &ps2_keyboard_irq, 1, 1, 0);
    pic_clear_mask(1);
}
