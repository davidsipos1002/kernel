#include <ps2/keyboard.h>

#include <asm/io.h>
#include <gcc/interrupt.h>
#include <graphics/print.h>
#include <interrupt/idt.h>
#include <interrupt/pic.h>
#include <ps2/controller.h>

void INTERRUPT ps2_keyboard_irq(no_priv_change_frame *stack_frame)
{
    pic_eoi(1);
    uint8_t data = io_in(PS2_DATA);
    if (data == 0xF0 || data == 0xE0) {
        data = io_in(PS2_DATA);
    }
}  

void ps2_keyboard_init(void *idt, uint8_t irq)
{
    idt_register_handler(idt, irq, (idt_handler) &ps2_keyboard_irq, 1, 1, 0);
    pic_clear_mask(1);
}
