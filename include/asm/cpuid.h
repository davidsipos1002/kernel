#ifndef ASM_CPUID_H_INCL
#define ASM_CPUID_H_INCL

#include <stdint.h>

typedef struct 
{
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
} cpuid_res;

void cpuid(const uint64_t rax, const uint64_t rcx, cpuid_res *result); 

#endif
