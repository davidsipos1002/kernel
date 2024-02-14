#ifndef SEGMENT_TSS_H_INCL
#define SEGMENT_TSS_H_INCL

#include <stdint.h>

#include <cpu/state.h>

typedef struct
{
    uint32_t reserved_0;
    uint32_t rsp[6];
    uint32_t reserved_1[2];
    uint32_t ist[14];
    uint32_t reserved_2[2];
    uint32_t io_map_base;
} task_state_segment;

void tss_fill(cpu_state *state);

#endif