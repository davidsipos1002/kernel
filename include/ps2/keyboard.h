#ifndef PS2_KEYBOARD_H_INCL
#define PS2_KEYBOARD_H_INCL

#include <stdint.h>

#define PS2_KEYBOARD_L_SHIFT 0x04
#define PS2_KEYBOARD_R_SHIFT 0x5C
#define PS2_KEYBOARD_CAPS_LOCK 0x03
#define PS2_KEYBOARD_ENTER 0x6A
#define PS2_KEYBOARD_BACKSPACE 0x69

typedef struct
{
    uint8_t code;
    uint8_t event;
    char ascii;
} ps2_key_event;

void ps2_keyboard_init(void *idt, uint8_t irq);
void ps2_keyboard_get_key(ps2_key_event *event);

#endif
