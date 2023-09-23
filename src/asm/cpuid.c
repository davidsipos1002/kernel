#include <asm/cpuid.h>

void cpuid(const uint64_t rax, const uint64_t rcx, cpuid_res *result) 
{
    __asm__ __volatile__ ("cpuid"
        : "=a" (result->rax), "=b" (result->rbx), "=c" (result->rcx), "=d" (result->rdx)
        : "a" (rax), "c" (rcx)
        : "memory");
}