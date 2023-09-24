#ifndef CPU_CONTROL_H_INCL
#define CPU_CONTROL_H_INCL

#include <stdint.h>

#define PAGING_CR0_WP (1 << 16)
#define PAGING_CR0_PG (1 << 31)
#define PAGING_CR4_PAE (1 << 5)
#define PAGING_CR4_PGE (1 << 7)
#define PAGING_CR4_LA57 (1 << 12)
#define PAGING_CR4_PCIDE (1 << 17)
#define PAGING_CR4_SMEP (1 << 20)
#define PAGING_CR4_SMAP (1 << 21)
#define PAGING_CR4_PKE (1 << 22)
#define PAGING_CR4_CET (1 << 23)
#define PAGING_CR4_PKS (1 << 24)
#define MODE_IA32_EFER_LME (1 << 8)
#define PAGING_IA32_EFER_NXE (1 << 11)
#define PAGING_IA32_MTRR_DEF_TYPE_E (1 << 11)
#define PAGING_RFLAGS_AC (1 << 18)

#define PAGING_PAT_UC 0x00
#define PAGING_PAT_WC 0x01
#define PAGING_PAT_WT 0x04
#define PAGING_PAT_WP 0x05
#define PAGING_PAT_WB 0x06
#define PAGING_PAT_UC_ 0x07

void set_cr4_flag(uint64_t flag);
void unset_cr4_flag(uint64_t flag);
void set_ia32_efer_flag(uint64_t flag);
void unset_ia32_efer_flag(uint64_t flag);

#endif