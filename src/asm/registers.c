#include <asm/registers.h>

uint64_t get_rflags()
{
    uint64_t ret;
    __asm__ __volatile__ ("pushfq; pop %0"
        : "=r" (ret)
        :
        : "memory");
    return ret;
}

void set_rflags(uint64_t rflags)
{
    __asm__ __volatile__ ("push %0; popfq"
        :
        : "r" (rflags)
        : "memory");
}

uint64_t get_cr0()
{
    uint64_t ret;
    __asm__ __volatile__ ("mov %%cr0, %0"
        : "=r" (ret)
        :
        : "memory");
    return ret;
}

void set_cr0(uint64_t cr0)
{
    __asm__ __volatile__ ("mov %0, %%cr0"
        :
        : "r" (cr0)
        : "memory");
}

uint64_t get_cr1()
{
    uint64_t ret;
    __asm__ __volatile__ ("mov %%cr1, %0"
        : "=r" (ret)
        :
        : "memory");
    return ret;
}

void set_cr1(uint64_t cr1)
{
    __asm__ __volatile__ ("mov %0, %%cr1"
        :
        : "r" (cr1)
        : "memory");
}

uint64_t get_cr2()
{
    uint64_t ret;
    __asm__ __volatile__ ("mov %%cr2, %0"
        : "=r" (ret)
        :
        : "memory");
    return ret;
}

void set_cr2(uint64_t cr2)
{
    __asm__ __volatile__ ("mov %0, %%cr2"
        :
        : "r" (cr2)
        : "memory");
}

uint64_t get_cr3() 
{
    uint64_t ret;
    __asm__ __volatile__ ("mov %%cr3, %0"
        : "=r" (ret)
        :
        : "memory");
    return ret;
}

void set_cr3(uint64_t cr3)
{
    __asm__ __volatile__ ("mov %0, %%cr3"
        :
        : "r" (cr3)
        : "memory");
}

uint64_t get_cr4()
{
    uint64_t ret;
    __asm__ __volatile__ ("mov %%cr4, %0"
        : "=r" (ret)
        :
        : "memory");
    return ret;
}

void set_cr4(uint64_t cr4)
{
    __asm__ __volatile__ ("mov %0, %%cr4"
        :
        : "r" (cr4)
        : "memory");
}

uint64_t get_cr8()
{
    uint64_t ret;
    __asm__ __volatile__ ("mov %%cr8, %0"
        : "=r" (ret)
        :
        : "memory");
    return ret;
}

void set_cr8(uint64_t cr8)
{
    __asm__ __volatile__ ("mov %0, %%cr8"
        :
        : "r" (cr8)
        : "memory");
}

uint64_t get_pkru()
{
    uint64_t ret;
    __asm__ __volatile__ ("xor %%rcx, %%rcx; rdpkru"
        : "=a" (ret)
        :
        : "memory", "rcx", "rdx");
    return ret;
}

void set_pkru(uint64_t pkru)
{
    __asm__ __volatile__ ("xor %%rcx, %%rcx; xor %%rdx, %%rdx; wrpkru"
        :
        : "a" (pkru)
        : "memory", "rcx", "rdx");
}

uint64_t get_msr(uint64_t reg)
{
    uint64_t rdx, rax;
    __asm__ __volatile__ ("rdmsr"
        : "=d" (rdx), "=a" (rax)
        : "c" (reg)
        : "memory");
    return ((rdx & UINT32_MAX) << 32) | (rax & UINT32_MAX);
}

void set_msr(uint64_t reg, uint64_t val)
{
    uint64_t rdx = (val &  ((uint64_t) UINT32_MAX << 32)) >> 32;
    uint64_t rax = val & UINT32_MAX;
    __asm__ __volatile__("wrmsr"
        :
        : "c" (reg), "d" (rdx), "a" (rax)
        : "memory");
}