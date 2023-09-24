#ifndef CPU_FEATURES_H_INCL
#define CPU_FEATURES_H_INCL

#define PAGING_RAX_01_RDX_PAE           (1 << 6)
#define PAGING_RAX_01_RDX_PGE           (1 << 13)
#define PAGING_RAX_01_RDX_PAT           (1 << 16)
#define PAGING_RAX_01_RDX_MTRR          (1 << 12)
#define PAGING_RAX_01_RCX_PCID          (1 << 17)
#define PAGING_RAX_07_RCX_0_RBX_SMEP    (1 << 7)
#define PAGING_RAX_07_RCX_0_RBX_SMAP    (1 << 20)
#define PAGING_RAX_07_RCX_0_RCX_PKU     (1 << 3)
#define PAGING_RAX_07_RCX_0_RCX_CET_SS  (1 << 7)
#define PAGING_RAX_07_RCX_0_RCX_PKS     (1 << 31)
#define PAGING_RAX_80000001_RDX_NX      (1 << 20)
#define PAGING_RAX_80000001_RDX_Page1GB (1 << 26)
#define PAGING_RAX_80000001_RDX_LM      (1 << 29)
#define PAGING_RAX_80000008_EAX_7_0     (0xFF)
#define PAGING_RAX_80000008_EAX_15_8    (0xFF00)

#endif