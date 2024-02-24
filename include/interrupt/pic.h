#ifndef INTERRUPT_PIC_H_INCL
#define INTERRUPT_PIC_H_INCL

#include <stdint.h>

#define PIC_M_COMMAND 0x20
#define PIC_M_DATA 0x21
#define PIC_S_COMMAND 0xA0
#define PIC_S_DATA 0xA1

#define PIC_ICW1 0b00010001
#define PIC_ICW3_M 0b00000100
#define PIC_ICW3_S 0b00000010
#define PIC_ICW4 0b00000001
#define PIC_EOI 0b00100000
#define PIC_OCW3_IRR 0b000001010
#define PIC_OCW3_ISR 0b000001011

void pic_init(uint8_t offset_m, uint8_t offset_s);
void pic_eoi(uint8_t irq);
void pic_set_mask(uint8_t irq);
void pic_clear_mask(uint8_t irq);
void pic_disable();
uint16_t pic_get_irr();
uint16_t pic_get_isr();

#endif
