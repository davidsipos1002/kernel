#ifndef PS2_KEYBOARD_H_INCL
#define PS2_KEYBOARD_H_INCL

#include <stdint.h>

void ps2_keyboard_init(void *idt, uint8_t irq);

#endif
