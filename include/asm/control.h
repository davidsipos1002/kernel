#ifndef ASM_CONTROL_H_INCL
#define ASM_CONTROL_H_INCL

#include <stdint.h>

void invlpg(void *p);
void wbinvd();
void ltr(uint16_t selector);
void lidt(void *base, uint16_t limit);
void cli();
void sti();
void pause();

#endif
