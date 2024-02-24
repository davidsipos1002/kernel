#include <interrupt/pic.h>

#include <asm/io.h>

void pic_init(uint8_t offset_m, uint8_t offset_s)
{
    uint8_t mask_m, mask_s;

    mask_m = io_in(PIC_M_DATA);
    mask_s = io_in(PIC_S_DATA);

    io_out(PIC_M_COMMAND, PIC_ICW1);
    io_wait();
    io_out(PIC_S_COMMAND, PIC_ICW1);
    io_wait();
    io_out(PIC_M_DATA, offset_m);
    io_wait();
    io_out(PIC_S_DATA, offset_s);
    io_wait();
    io_out(PIC_M_DATA, PIC_ICW3_M);
    io_wait();
    io_out(PIC_S_DATA, PIC_ICW3_S); 
    io_wait();
    io_out(PIC_M_DATA, PIC_ICW4);
    io_wait();
    io_out(PIC_S_DATA, PIC_ICW4);

    io_out(PIC_M_DATA, mask_m);
    io_out(PIC_S_DATA, mask_s);
}

void pic_eoi(uint8_t irq)
{
    if (irq >= 8)
        io_out(PIC_S_COMMAND, PIC_EOI);
    io_out(PIC_M_COMMAND, PIC_EOI); 
} 

void pic_set_mask(uint8_t irq)
{
    uint16_t port;
    uint8_t data;
    if (irq < 8)
        port = PIC_M_DATA;
    else {
        port = PIC_S_DATA;
        irq -= 8;
    }
    data = io_in(port) | (1 << irq);
    io_out(port, data);
}

void pic_clear_mask(uint8_t irq)
{
    uint16_t port;
    uint8_t data;
    if (irq < 8)
        port = PIC_M_DATA;
    else {
        port = PIC_S_DATA;
        irq -= 8;
    }
    data = io_in(port) & (~(1 << irq));
    io_out(port, data);
}

void pic_disable()
{
    io_out(PIC_M_DATA, 0xFF);
    io_out(PIC_S_DATA, 0xFF);
}

uint16_t pic_get_irr()
{
    io_out(PIC_M_COMMAND, PIC_OCW3_IRR);
    io_out(PIC_S_COMMAND, PIC_OCW3_IRR);
    return ((uint16_t) io_in(PIC_S_COMMAND) << 8) | io_in(PIC_M_COMMAND);  
}

uint16_t pic_get_isr()
{
    io_out(PIC_M_COMMAND, PIC_OCW3_IRR);
    io_out(PIC_S_COMMAND, PIC_OCW3_IRR);
    return ((uint16_t) io_in(PIC_S_COMMAND) << 8) | io_in(PIC_M_COMMAND);  
}
