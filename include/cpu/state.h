#ifndef CPU_STATE_H_INCL
#define CPU_STATE_H_INCL

#include <stdint.h>

typedef struct
{
     void *gdt;
     void *tss;
     void *idt;
     uint16_t io_map_base;
     uint8_t ist_count;
     uint64_t ist[7];
} cpu_state;

#endif