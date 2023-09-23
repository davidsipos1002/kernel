#include <boot/bootinfo.h>
#include <asm/cpuid.h>

int kernel_main(BootInfo *bootInfo) 
{
    cpuid_res res;
    cpuid(7, 0, &res);
    while (1);
    return 0;
}