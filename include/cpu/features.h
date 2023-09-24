#ifndef CPU_FEATURES_H_INCL
#define CPU_FEATURES_H_INCL

#define PAGING_RAX_01_RDX_PAE           ((uint64_t) 1 << 6)
#define PAGING_RAX_01_RDX_PGE           ((uint64_t) 1 << 13)
#define PAGING_RAX_01_RDX_PAT           ((uint64_t) 1 << 16)
#define PAGING_RAX_01_RDX_MTRR          ((uint64_t) 1 << 12)
#define PAGING_RAX_01_RCX_PCID          ((uint64_t) 1 << 17)
#define PAGING_RAX_07_RCX_0_RBX_SMEP    ((uint64_t) 1 << 7)
#define PAGING_RAX_07_RCX_0_RBX_SMAP    ((uint64_t) 1 << 20)
#define PAGING_RAX_07_RCX_0_RCX_PKU     ((uint64_t) 1 << 3)
#define PAGING_RAX_07_RCX_0_RCX_CET_SS  ((uint64_t) 1 << 7)
#define PAGING_RAX_07_RCX_0_RCX_PKS     ((uint64_t) 1 << 31)
#define PAGING_RAX_80000001_RDX_NX      ((uint64_t) 1 << 20)
#define PAGING_RAX_80000001_RDX_Page1GB ((uint64_t) 1 << 26)
#define PAGING_RAX_80000001_RDX_LM      ((uint64_t) 1 << 29)
#define PAGING_RAX_80000008_EAX_7_0     (0xFF)
#define PAGING_RAX_80000008_EAX_15_8    (0xFF00)

#endif