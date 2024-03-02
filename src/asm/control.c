#include <asm/control.h>
#include <stdint.h>

void invlpg(void *p)
{
    __asm__ __volatile__ ("invlpg (%0)"
        :
        : "r" ((uint64_t) p)
        : "memory");
}

void wbinvd()
{
    __asm__ __volatile__ ("wbinvd"
        :
        :
        : "memory");
}

void ltr(uint16_t selector)
{
    __asm__ __volatile__("ltr %0"
        :
        : "r" (selector)
        : "memory");
}

void lidt(void *base, uint16_t limit)
{
    uint8_t idtr[10];
    *((uint16_t *) idtr) = limit;
    *((uint64_t *) &idtr[2]) = (uint64_t) base;
    __asm__ __volatile__("lidt (%0)"
        :
        : "r" ((uint64_t) idtr)
        : "memory");
}

void sgdt(volatile void *addr)
{
    __asm__ __volatile__ ("sgdt (%0)"
        :
        : "r"(addr)
        : "memory");
}

void sidt(volatile void *addr)
{
    __asm__ __volatile__ ("sidt (%0)"
        :
        : "r"(addr)
        : "memory");
}

uint16_t str()
{
    uint16_t ret;
    __asm__ __volatile__ ("str %0"
        : "=r" (ret)
        :
        : "memory");
    return ret;
}

void cli()
{
    __asm__ __volatile__ ("cli"
        :
        :
        : "memory");
}

void sti()
{
    __asm__ __volatile__ ("sti"
        :
        :
        : "memory");
}

void pause()
{
    __asm__ __volatile__ ("pause"
        :
        :
        : "memory");
}