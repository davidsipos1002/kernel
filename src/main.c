#include <asm/registers.h>
#include <boot/bootinfo.h>
#include <memory/manipulate.h>
#include <paging/paging.h>

extern char __kernel_data_begin[];
extern char __kernel_data_end[];

int kernel_main(BootInfo *bootInfo) 
{
    paging_state* state = paging_init(get_cr3(), (void *) __kernel_data_begin);
    memcpy((void *) (__kernel_data_begin + sizeof(paging_state)), bootInfo, sizeof(BootInfo)); 
    while (1);
    return 0;
}