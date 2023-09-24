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