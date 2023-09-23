#ifndef ASM_REGISTERS_H_INCL
#define ASM_REGISTERS_H_INCL

#include <stdint.h>

uint64_t get_rflags();
void set_rflags(uint64_t rflags);

uint64_t get_cr0();
void set_cr0(uint64_t cr0);

uint64_t get_cr1();
void set_cr1(uint64_t cr1);

uint64_t get_cr2();
void set_cr2(uint64_t cr2);

uint64_t get_cr3();
void set_cr3(uint64_t cr3);

uint64_t get_cr4();
void set_cr4(uint64_t cr4);

uint64_t get_cr8();
void set_cr8(uint64_t cr8);

uint64_t get_pkru();
void set_pkru(uint64_t pkru);

uint64_t get_msr(uint64_t reg);
void set_msr(uint64_t reg, uint64_t val);

#endif