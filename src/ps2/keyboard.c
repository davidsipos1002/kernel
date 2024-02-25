#include <ps2/keyboard.h>

#include <algorithm/bit_field.h>
#include <asm/control.h>
#include <asm/io.h>
#include <gcc/interrupt.h>
#include <gcc/utils.h>
#include <graphics/print.h>
#include <interrupt/idt.h>
#include <interrupt/pic.h>
#include <memory/manipulate.h>
#include <ps2/controller.h>

#define KEYMAP_ADDR 0x18FFD000
#define SHIFT 0x12
typedef struct
{
    uint8_t normal[256];
    uint8_t emap[256];
    uint16_t ascii[256];
} PACKED_STRUCT key_map; 

static key_map *keymap = (key_map *) KEYMAP_ADDR;
static uint8_t keystat[32];
static uint8_t event_occured;
static ps2_key_event curr_event;

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
    
    char ascii = 0;
    uint8_t shift_pos = keymap->normal[SHIFT];
    if (bit_field_get_bit(keystat, shift_pos))
        ascii = (keymap->ascii[pos] >> 8) & 0xFF;
    else
        ascii = keymap->ascii[pos] & 0xFF;
    curr_event.position = pos;
    curr_event.released = released;
    curr_event.ascii = ascii;
    event_occured = 1;
    pic_eoi(1);
}

void ps2_keyboard_init(void *idt, uint8_t irq)
{
    memset(keystat, 0, 32);
    idt_register_handler(idt, irq, (idt_handler) &ps2_keyboard_irq, 1, 1, 0);
    pic_clear_mask(1);
}

void ps2_keyboard_get_key(ps2_key_event *event)
{
    while (!event_occured);
    cli();
    event_occured = 0;
    event->position = curr_event.position;
    event->released = curr_event.released;
    event->ascii = curr_event.ascii;
    sti();
}