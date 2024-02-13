#ifndef SEGMENT_GDT_H_INCL
#define SEGMENT_GDT_H_INCL

#include <stdint.h>

#include <segment/format.h>

void segment_fill_gdt(void *gdt);
void segment_set_gdt(void *gdt);

#endif