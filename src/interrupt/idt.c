#include <interrupt/idt.h>

#include <memory/manipulate.h>

void idt_register_handler(void *idt, uint8_t vector, idt_handler handler, uint8_t ist, uint8_t interrupt, uint8_t dpl)
{
    idt_gate_descriptor *desc = &((idt_gate_descriptor *) idt)[vector];
    memset(desc, 0, sizeof(idt_gate_descriptor));
    uint64_t offset = (uint64_t) handler; 
    desc->offset_0 = offset & 0xFFFF;
    offset >>= 16;
    desc->selector = 0x0008; 
    desc->ist = ist & 0x07;
    desc->type = 14 + !interrupt;
    desc->dpl = dpl & 0x03;
    desc->p = 1;
    desc->offset_1 = offset & 0xFFFF;
    offset >>= 16;
    desc->offset_2 = offset;
}