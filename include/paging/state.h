#ifndef PAGING_STATE_H_INCL
#define PAGING_STATE_H_INCL

#include <stdint.h>

#define PAGING_PAGE_SIZE 4096
#define PAGING_PAGE_SIZE_EXP 12

#define PAGING_DISABLED 0
#define PAGING_4_LEVEL 4
#define PAGING_5_LEVEL 5

#define PAGING_GLOBAL_PAGES_DISABLED 0
#define PAGING_GLOBAL_PAGES_ENABLED 1

#define PAGING_PAT_NOT_SUPPORTED 0
#define PAGING_PAT_SUPPORTED 1

#define PAGING_PCID_DISABLED 0
#define PAGING_PCID_ENABLED 1

#define PAGING_SMEP_DISABLED 0
#define PAGING_SMEP_ENABLED 1

#define PAGING_SMAP_DISABLED 0
#define PAGING_SMAP_ENABLED 1

#define PAGING_PKU_DISABLED 0
#define PAGING_PKU_ENABLED 1

#define PAGING_PKS_DISABLED 0
#define PAGING_PKS_ENABLED 1

#define PAGING_NX_DISABLED 0
#define PAGING_NX_ENABLED 1

#define PAGING_MTRR_DISABLED 0
#define PAGING_MTRR_ENABLED 1

#define PAGING_FLAG_UNSET 0
#define PAGING_FLAG_SET 1

typedef struct
{
    uint8_t mode;
    uint8_t pg;
    uint8_t pat; 
    uint8_t pcid;
    uint8_t smep;
    uint8_t smap;
    uint8_t pku;
    uint8_t pks;
    uint8_t nx;
    uint8_t mtrr;
    uint8_t wp;
    uint8_t rflags_ac;
    uint8_t phys_addr_width;
    uint8_t lin_addr_width;
} paging_state;

#endif