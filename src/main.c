#include <asm/control.h>
#include <asm/cpuid.h>
#include <asm/registers.h>
#include <boot/bootinfo.h>
#include <cpu/control.h>
#include <cpu/features.h>
#include <cpu/register_id.h>
#include <paging/format.h>
#include <paging/control.h>
#include <paging/paging.h>

extern char __kernel_data_begin[];
extern char __kernel_data_end[];

int kernel_main(BootInfo *bootInfo) 
{
    paging_state* state = paging_init(get_cr3(), (void *) __kernel_data_begin);
    while (1);
    return 0;
}