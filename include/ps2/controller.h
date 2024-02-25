#ifndef PS2_CONTROLLER_H_INCL
#define PS2_CONTROLLER_H_INCL

#include <stdint.h>

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64

uint8_t ps2_controller_init();
void ps2_write(uint8_t port, uint8_t data);
uint8_t ps2_read(uint8_t port);

#endif
