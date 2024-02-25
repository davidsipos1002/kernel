#ifndef GCC_INTERRUPT_H_INCL
#define GCC_INTERRUPT_H_INCL

#include <stdint.h>

typedef struct 
{
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
} no_priv_change_frame;

#endif
