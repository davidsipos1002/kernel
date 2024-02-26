#ifndef PS2_CONTROLLER_H_INCL
#define PS2_CONTROLLER_H_INCL

#include <stdint.h>

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64

#define PS2_NOT_A_KEYBOARD 1
#define PS2_DISABLE_SCAN_FAIL 2
#define PS2_CONTROLLER_SELF_TEST_FAIL 3
#define PS2_PORT_TEST_FAILED 4
#define PS2_RESET_FAILED 5
#define PS2_DEVICE_TEST_FAILED 6
#define PS2_IDENTIFY_FAILED 7
#define PS2_ENABLE_SCAN_FAIL 8
#define PS2_SUCCESS 0

uint8_t ps2_controller_init();
void ps2_write(uint8_t port, uint8_t data);
uint8_t ps2_read(uint8_t port);

#endif
