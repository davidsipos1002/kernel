#ifndef ASM_IO_H_INCL
#define ASM_IO_H_INCL

#include <stdint.h>

void io_out(uint16_t port, uint8_t data);
uint8_t io_in(uint16_t port);
void io_wait();

#endif
