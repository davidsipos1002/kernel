#include <paging/control.h>

#include <asm/cpuid.h>
#include <asm/registers.h>
#include <cpu/control.h>
#include <cpu/features.h>
#include <cpu/register_id.h>

void enable_4_level_paging(uint64_t cr3)
{
    uint64_t reg = get_cr4();
    reg |= PAGING_CR4_PAE;
    set_cr4(reg);
    set_cr3(cr3);
    reg = get_msr(IA32_EFER);
    reg |= MODE_IA32_EFER_LME;
    set_msr(IA32_EFER, reg);
    reg = get_cr0();
    reg |= PAGING_CR0_PG;
    set_cr0(reg);
}

void disable_4_level_paging()
{
    uint64_t reg = get_cr0();
    reg &= ~PAGING_CR0_PG;
    set_cr0(reg);
}

uint8_t enable_global_pages()
{
    cpuid_res id;
    cpuid(1, 0, &id);
    if (!(id.rdx & PAGING_RAX_01_RDX_PGE))
        return 0;
    set_cr4_flag(PAGING_CR4_PGE);
    return 1;
}

uint8_t disable_global_pages()
{
    unset_cr4_flag(PAGING_CR4_PGE); 
    return 1;
}

uint8_t is_pat_supported()
{
    cpuid_res id; 
    cpuid(1, 0, &id);
    return (id.rdx & PAGING_RAX_01_RDX_PAT) != 0;
}

uint8_t enable_pcid()
{
    cpuid_res id;
    cpuid(1, 0, &id);
    if (!(id.rcx & PAGING_RAX_01_RCX_PCID))
        return 0; 
    set_cr4_flag(PAGING_CR4_PCIDE);
    return 1;
}

uint8_t disable_pcid()
{
    unset_cr4_flag(PAGING_CR4_PCIDE);
    return 1;
}

uint8_t enable_smep()
{
    cpuid_res id;
    cpuid(7, 0, &id);
    if (!(id.rbx & PAGING_RAX_07_RCX_0_RBX_SMEP))
        return 0;
    set_cr4_flag(PAGING_CR4_SMEP);
    return 1;
}

uint8_t disable_smep()
{
    unset_cr4_flag(PAGING_CR4_SMEP);
    return 1;
}

uint8_t enable_smap()
{
    cpuid_res id;
    cpuid(7, 0, &id);
    if (!(id.rbx & PAGING_RAX_07_RCX_0_RBX_SMAP))
        return 0;
    set_cr4_flag(PAGING_CR4_SMAP);
    return 1;
}

uint8_t disable_smap()
{
    unset_cr4_flag(PAGING_CR4_SMAP);
    return 1;
}

uint8_t enable_pku()
{
    cpuid_res id;
    cpuid(7, 0, &id);
    if (!(id.rcx & PAGING_RAX_07_RCX_0_RCX_PKU))
        return 0;
    set_cr4_flag(PAGING_CR4_PKE);
    return 1;
}

uint8_t disable_pku()
{
    unset_cr4_flag(PAGING_CR4_PKE);
    return 1;
}

uint8_t enable_pks()
{
    cpuid_res id;
    cpuid(7, 0, &id);
    if (!(id.rcx & PAGING_RAX_07_RCX_0_RCX_PKS))
        return 0;
    set_cr4_flag(PAGING_CR4_PKS);
    return 1;
}

uint8_t disable_pks()
{
    unset_cr4_flag(PAGING_CR4_PKS);
    return 1;
}

uint8_t enable_nx()
{
    cpuid_res id;
    cpuid(0x80000001, 0, &id);
    if (!(id.rdx & PAGING_RAX_80000001_RDX_NX))
        return 0;
    set_ia32_efer_flag(PAGING_IA32_EFER_NXE);
    return 1;
}

uint8_t disable_nx()
{
    unset_ia32_efer_flag(PAGING_IA32_EFER_NXE);
    return 1;
}

void get_address_widths(uint8_t *physical, uint8_t *linear)
{
    cpuid_res id;
    cpuid(0x80000008, 0, &id);
    *physical = (uint8_t) (id.rax & PAGING_RAX_80000008_EAX_7_0);
    *linear = (uint8_t) ((id.rax & PAGING_RAX_80000008_EAX_15_8) >> 8);
}

uint8_t disable_mtrr()
{
    cpuid_res id;
    cpuid(1, 0, &id);
    if (!(id.rdx & PAGING_RAX_01_RDX_MTRR))
        return 0;
    uint64_t ia32_mtrr_def_type = get_msr(IA32_MTRR_DEF_TYPE);
    ia32_mtrr_def_type &= ~PAGING_IA32_MTRR_DEF_TYPE_E;
    set_msr(IA32_MTRR_DEF_TYPE, ia32_mtrr_def_type);
    return 1;
}

uint64_t get_pat()
{
    return get_msr(IA32_PAT);
}

void set_pat(uint64_t pat)
{
    set_msr(IA32_PAT, pat);
}

void format_pat_entry(uint64_t *pat, uint8_t entry, uint8_t type)
{
    uint64_t mask = (uint64_t) 0xFF << (entry << 3);
    uint64_t field = (uint64_t) type << (entry << 3); 
    *pat &= ~mask;
    *pat |= field;
}

uint64_t get_pkr(uint8_t supervisor)
{
    if (supervisor)
        return get_msr(IA32_PKRS);
    else
        return get_pkru();
}

void set_pkr(uint8_t supervisor, uint64_t pkr)
{
    if (supervisor)
        set_msr(IA32_PKRS, pkr);
    else
        set_pkru(pkr);
}

void format_pkr(uint64_t *pkr, uint8_t key, uint8_t write_disable, uint8_t access_disable)
{
    uint64_t new_rights = (uint64_t) (((write_disable != 0) << 1) | (access_disable != 0)) << (key << 1);
    uint64_t mask = (uint64_t) 0x3 << (key << 1);
    *pkr &= ~mask;
    *pkr |= new_rights;
}

void set_rflags_ac()
{
    uint64_t rflags = get_rflags();
    rflags |= PAGING_RFLAGS_AC;
    set_rflags(rflags);
}

void unset_rflags_ac()
{
    uint64_t rflags = get_rflags();
    rflags &= ~PAGING_RFLAGS_AC;
    set_rflags(rflags);
}

void set_wp()
{
    uint64_t cr0 = get_cr0();
    cr0 |= PAGING_CR0_WP;
    set_cr0(cr0);
}

void unset_wp()
{
    uint64_t cr0 = get_cr0();
    cr0 &= ~PAGING_CR0_WP;
    set_cr0(cr0);
}