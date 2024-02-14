#ifndef SEGMENT_GDT_H_INCL
#define SEGMENT_GDT_H_INCL

#include <stdint.h>

#include <cpu/state.h>
#include <segment/format.h>

void segment_fill_gdt(cpu_state *state);
void segment_set_gdt(cpu_state *state);
void segment_fill_tss(cpu_state *state);

#endif