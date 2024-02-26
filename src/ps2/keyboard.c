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
typedef struct
{
    uint8_t normal[256];
    uint8_t emap[256];
    uint16_t ascii[256];
} PACKED_STRUCT key_map; 

static key_map *keymap = (key_map *) KEYMAP_ADDR;
static uint8_t extended;
static uint8_t keystat[32];
static uint8_t event_occured;
static ps2_key_event curr_event;

static inline void ALWAYS_INLINE getbuff(char *buff, uint8_t data)
{
    buff[0] = '0';
    buff[1] = 'x';
    uint8_t first = (data >> 4) & 0xF;
    uint8_t second = data & 0xF;
    buff[2] = first >= 10 ? ('A' + first - 10) : ('0' + first);
    buff[3] = second >= 10 ? ('A' + second - 10) : ('0' + second);
    buff[4] = '\0';
}

void INTERRUPT ps2_keyboard_irq(no_priv_change_frame *stack_frame)
{
    uint8_t data = io_in(PS2_DATA);

    if (data == 0xE0)
        extended = 1;
    else
    {
        uint8_t released = ((data & 0x80) != 0);
        data &= 0x7F;
        uint8_t code = extended ? keymap->emap[data] : keymap->normal[data];
        extended = 0;

        if (code != PS2_KEYBOARD_CAPS_LOCK) 
        {
            if (released)
                bit_field_unset_bit(keystat, code);
            else
                bit_field_set_bit(keystat, code);
        } 
        else if (released)
            bit_field_toggle_bit(keystat, code);

        uint8_t shift_cond = bit_field_get_bit(keystat, PS2_KEYBOARD_CAPS_LOCK) || 
            bit_field_get_bit(keystat, PS2_KEYBOARD_L_SHIFT) || bit_field_get_bit(keystat, PS2_KEYBOARD_R_SHIFT);
        char ascii = 0;
        if (shift_cond)
            ascii = (keymap->ascii[code] >> 8) & 0xFF;
        else
            ascii = keymap->ascii[code] & 0xFF;
    
        event_occured = 1;
        curr_event.code = code;
        curr_event.released = released;
        curr_event.ascii = ascii;
    }

    pic_eoi(1);
}

void ps2_keyboard_init(void *idt, uint8_t irq)
{
    idt_register_handler(idt, irq, (idt_handler) &ps2_keyboard_irq, 1, 1, 0);
    pic_clear_mask(1);
}

void ps2_keyboard_get_key(ps2_key_event *event)
{
    while (!event_occured);
    cli();
    event_occured = 0;
    event->code = curr_event.code;
    event->released = curr_event.released;
    event->ascii = curr_event.ascii;
    sti();
}