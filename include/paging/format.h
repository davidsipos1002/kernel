#ifndef PAGING_FORMAT_H_INCL
#define PAGING_FORMAT_H_INCL

#include <stdint.h>
#include <gcc/utils.h>

#define PAGING_PML4_INDEX_MASK  0x00000FF8000000000 
#define PAGING_PML4_INDEX_SHIFT 39
#define PAGING_PDP_INDEX_MASK   0x00000007FC0000000
#define PAGING_PDP_INDEX_SHIFT  30
#define PAGING_PD_INDEX_MASK    0x0000000003FE00000
#define PAGING_PD_INDEX_SHIFT   21
#define PAGING_PT_INDEX_MASK    0x000000000001FF000
#define PAGING_PT_INDEX_SHIFT   12
#define PAGING_PAGE_OFFSET_MASK 0x00000000000000FFF

typedef struct
{
    uint64_t ignored : 3;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t ignored1 : 7;
    uint64_t address : 40;
    uint64_t reserved : 12;
} PACKED_STRUCT cr3_no_pcide;

typedef struct 
{
    uint64_t pcid : 12;
    uint64_t address : 40;
    uint64_t reserved : 12;
} PACKED_STRUCT cr3_pcide;

typedef struct
{
    uint32_t p : 1;
    uint32_t w_r : 1;
    uint32_t u_s : 1;
    uint32_t rsvd : 1;
    uint32_t i_d : 1;
    uint32_t pk : 1;
    uint32_t ss : 1;
    uint32_t hlat : 1;
    uint32_t reserved : 7;
    uint32_t sgx : 1;
    uint32_t reserved1 : 16;
} PACKED_STRUCT pf_errcode; 

typedef struct
{
    uint64_t p : 1;
    uint64_t r_w : 1;
    uint64_t u_s : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t a : 1;
    uint64_t ignored : 1;
    uint64_t ps : 1;
    uint64_t ignored1 : 4;
    uint64_t address : 40;
    uint64_t ignored2 : 11;
    uint64_t xd : 1;
} PACKED_STRUCT pml4e;

typedef struct 
{
    uint64_t p : 1;
    uint64_t r_w : 1;
    uint64_t u_s : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t a : 1;
    uint64_t d : 1;
    uint64_t ps : 1; //must be 1
    uint64_t g : 1;
    uint64_t ignored : 3;
    uint64_t pat : 1;
    uint64_t reserved : 17;
    uint64_t address : 22;
    uint64_t ignored1 : 7;
    uint64_t pk : 4;
    uint64_t xd : 1;
} PACKED_STRUCT pdpte_1gb;

typedef struct
{
    uint64_t p : 1;
    uint64_t r_w : 1;
    uint64_t u_s : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t a : 1;
    uint64_t ignored : 1;
    uint64_t ps : 1; // must be 0
    uint64_t ignored1 : 4;
    uint64_t address : 40;
    uint64_t ignored3 : 11;
    uint64_t xd : 1;
} PACKED_STRUCT pdpte;

typedef struct
{
    uint64_t p : 1;
    uint64_t r_w : 1;
    uint64_t u_s : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t a : 1;
    uint64_t d : 1;
    uint64_t ps : 1; // must be 1
    uint64_t g : 1;
    uint64_t ignored : 3;
    uint64_t pat : 1;
    uint64_t reserved : 8;
    uint64_t address : 31;
    uint64_t ignored1 : 7;
    uint64_t pk : 4;
    uint64_t xd : 1;
} PACKED_STRUCT pde_2mb;

typedef struct
{
    uint64_t p : 1;
    uint64_t r_w : 1;
    uint64_t u_s : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t a : 1;
    uint64_t ignored : 1;
    uint64_t ps : 1;
    uint64_t ignored1 : 4;
    uint64_t address : 40;
    uint64_t ignored2 : 11;
    uint64_t xd : 1;
} PACKED_STRUCT pde;

typedef struct
{
    uint64_t p : 1;
    uint64_t r_w : 1;
    uint64_t u_s : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t a : 1;
    uint64_t d : 1;
    uint64_t pat : 1;
    uint64_t g : 1;
    uint64_t ignored : 3;
    uint64_t address : 40;
    uint64_t ignored1 : 7;
    uint64_t pk : 4;
    uint64_t xd : 1;
} PACKED_STRUCT pte;

#endif