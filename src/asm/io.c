#include <asm/io.h>

void io_out(uint16_t port, uint8_t data)
{
    __asm__ __volatile__ ("outb %b0, %1"
        :
        : "a"(data), "d"(port)
        : "memory");
}

uint8_t io_in(uint16_t port)
{
    uint8_t data;
    __asm__ __volatile__("inb %1, %b0"
        : "=a"(data)
        : "d"(port)
        : "memory");
    return data;
}

void io_wait()
{
    io_out(0x80, 0);
}
