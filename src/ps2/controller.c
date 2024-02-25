#include <ps2/controller.h>

#include <asm/io.h>
#include <gcc/utils.h>

static inline void ALWAYS_INLINE ps2_out_wait()
{
    while (io_in(PS2_STATUS) & 0x2); 
}

static inline void ALWAYS_INLINE ps2_in_wait()
{
    while (!(io_in(PS2_STATUS) & 0x1));
}

void ps2_write(uint8_t port, uint8_t data)
{
    ps2_out_wait();
    io_out(port, data);
}

uint8_t ps2_read(uint8_t port)
{
    ps2_in_wait();
    return io_in(port);
}

static inline uint8_t ALWAYS_INLINE ps2_identify_keyboard()
{
    ps2_write(PS2_DATA, 0xF5); // disable scanning
    uint8_t resp = ps2_read(PS2_DATA); // ACK
    if (resp != 0xFA)
        return 0;
    ps2_write(PS2_DATA, 0xF2); // idenitfy device
    resp = ps2_read(PS2_DATA); // ACK
    if (resp != 0xFA)
        return 0;
    uint8_t type = 0;
    type = ps2_read(PS2_DATA);
    if (type != 0xAB && type != 0xAC) // not a keyboard
        return 0;
    type = io_in(PS2_DATA);
    ps2_write(PS2_DATA, 0xF4); // enable scanning
    resp = ps2_read(PS2_DATA);
    if (resp != 0xFA)
        return 0;
    io_in(PS2_DATA);
    return 1;
}

uint8_t ps2_controller_init()
{
    ps2_write(PS2_COMMAND, 0xAD); // disable first port
    ps2_write(PS2_COMMAND, 0xA7); // disable second port
    io_in(PS2_DATA); // flush output buffer
    
    // read config byte
    ps2_write(PS2_COMMAND, 0x20);
    uint8_t config = ps2_read(PS2_DATA);
    config &= ~((1 << 6) | 3); // disable interrupts and clear translation bit
    ps2_write(PS2_COMMAND, 0x60); // write config byte
    ps2_write(PS2_DATA, config);
    
    ps2_write(PS2_COMMAND, 0xAA); // perform controller self test
    uint8_t result = ps2_read(PS2_DATA);
    if (result != 0x55) // check if test failed 
        return 0;
    
    ps2_write(PS2_COMMAND, 0x60); // rewrite config byte
    ps2_write(PS2_DATA, config);
    
    ps2_write(PS2_COMMAND, 0xAB); // test first PS/2 port
    result = ps2_read(PS2_DATA);
    if (result) // check if test failed
        return 0;
    
    ps2_write(PS2_COMMAND, 0xAE); // enable first PS/2 port and enable interrupt
    config |= 1;
    ps2_write(PS2_COMMAND, 0x60);
    ps2_write(PS2_DATA, config);
    
    ps2_write(PS2_DATA, 0xFF); // send PS/2 reset to device
    result = ps2_read(PS2_DATA);
    if (result != 0xFA) // check if ACK
        return 0;
    result = ps2_read(PS2_DATA);
    if (result != 0xAA) // check if test passed
        return 0;

    return ps2_identify_keyboard();
}