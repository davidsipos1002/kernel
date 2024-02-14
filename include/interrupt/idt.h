#ifndef INTERRUPT_IDT_H_INCL
#define INTERRUPT_IDT_H_INCL

#include <stdint.h>

#include <gcc/utils.h>

typedef struct
{
    uint64_t offset_0 : 16;
    uint64_t selector : 16;
    uint64_t ist : 3;
    uint64_t zeros : 5;
    uint64_t type : 4;
    uint64_t zero : 1;
    uint64_t dpl : 2;
    uint64_t p : 1;
    uint64_t offset_1 : 16;
    uint64_t offset_2 : 32;
    uint64_t reserved : 32;
} PACKED_STRUCT idt_gate_descriptor;

typedef void (*idt_handler) (void);

void idt_register_handler(void *idt, uint8_t vector, idt_handler handler, uint8_t ist, uint8_t interrupt, uint8_t dpl);

#endif