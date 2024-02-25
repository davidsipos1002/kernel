#ifndef PS2_KEYBOARD_H_INCL
#define PS2_KEYBOARD_H_INCL

#include <stdint.h>

typedef struct
{
    uint8_t position;
    uint8_t released;
    char ascii;
} ps2_key_event;

void ps2_keyboard_init(void *idt, uint8_t irq);
void ps2_keyboard_get_key(ps2_key_event *event);

#endif
