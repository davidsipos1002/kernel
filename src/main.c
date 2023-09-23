#include <asm/control.h>
#include <asm/cpuid.h>
#include <asm/registers.h>
#include <boot/bootinfo.h>
#include <cpu/control.h>
#include <cpu/features.h>
#include <cpu/register_id.h>
#include <paging/format.h>

int kernel_main(BootInfo *bootInfo) 
{
    cpuid_res res;
    cpuid(7, 0, &res);
    uint64_t sz = sizeof(cr3_no_pcide);
    uint64_t sz1 = sizeof(cr3_pcide);
    uint64_t cr3 = get_cr3();
    cr3_no_pcide cr3_struct = *((cr3_no_pcide *) &cr3);
    set_cr3(cr3);
    uint64_t new_cr3 = get_cr3();
    uint64_t efer = get_msr(0xC0000080);
    efer |= 1;
    set_msr(0xC0000080, efer);
    invlpg((void *) &efer);
    cpuid(1, 0, &res);
    if((res.rdx & PAGING_RAX_01_RDX_PAT) && (res.rdx & PAGING_RAX_01_RDX_MTRR))
    {
        invlpg((void *) &res);
        uint64_t mtrr = get_msr(IA32_MTRR_DEF_TYPE);
        mtrr &= ~PAGING_IA32_MTRR_DEF_TYPE_E;
        set_msr(IA32_MTRR_DEF_TYPE, mtrr); 
    }
    while (1);
    return 0;
}