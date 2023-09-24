#include <cpu/control.h>

#include <asm/registers.h>
#include <cpu/register_id.h>

void set_cr4_flag(uint64_t flag)
{
    uint64_t cr4 = get_cr4();
    cr4 |= flag;
    set_cr4(cr4);
}

void unset_cr4_flag(uint64_t flag)
{
    uint64_t cr4 = get_cr4();
    cr4 &= ~flag;
    set_cr4(cr4);
}

void set_ia32_efer_flag(uint64_t flag)
{
    uint64_t ia32_efer = get_msr(IA32_EFER);
    ia32_efer |= flag;
    set_msr(IA32_EFER, ia32_efer);
}

void unset_ia32_efer_flag(uint64_t flag)
{
    uint64_t ia32_efer = get_msr(IA32_EFER);
    ia32_efer &= ~flag;
    set_msr(IA32_EFER, ia32_efer);
}