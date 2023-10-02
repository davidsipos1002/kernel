#include <paging/paging.h>

#include <asm/control.h>
#include <cpu/control.h>
#include <gcc/utils.h>
#include <memory/manipulate.h>
#include <paging/control.h>
#include <paging/format.h>

// Initialize paging
paging_state* paging_init(uint64_t cr3, void *state_location)
{
    paging_state *state = (paging_state *) state_location;
    
    //Setup kernel supported paging features
    enable_4_level_paging(cr3);
    state->mode = PAGING_4_LEVEL;
    state->pg = enable_global_pages();
    state->pat = is_pat_supported(); 
    state->pcid = enable_pcid();
    state->smep = enable_smep();
    state->smap = enable_smap();
    state->pku = enable_pku();
    state->pks = enable_pks();
    state->nx = enable_nx();
    get_address_widths(&(state->phys_addr_width), &(state->lin_addr_width));
    state->mtrr = !disable_mtrr();

    //Setup the PAT
    if (state->pat == PAGING_PAT_SUPPORTED)
    {
        uint64_t pat = 0;
        format_pat_entry(&pat, 0, PAGING_PAT_WB);
        format_pat_entry(&pat, 1, PAGING_PAT_WT);
        format_pat_entry(&pat, 2, PAGING_PAT_UC_);
        format_pat_entry(&pat, 3, PAGING_PAT_UC);
        format_pat_entry(&pat, 4, PAGING_PAT_WB);
        format_pat_entry(&pat, 5, PAGING_PAT_WT);
        format_pat_entry(&pat, 6, PAGING_PAT_WC);
        format_pat_entry(&pat, 7, PAGING_PAT_WP);
        set_pat(pat);
    } 

    // Setup protection key registers
    if (state->pku == PAGING_PKU_ENABLED || state->pks == PAGING_PKS_ENABLED)
    {
        uint64_t pkr = 0;
        for (uint8_t i = 0; i < 16; i++)
            format_pkr(&pkr, i, i & 0x2, i & 0x1);
        if(state->pku == PAGING_PKU_ENABLED)
            set_pkr(0, pkr);
        if(state->pks == PAGING_PKS_ENABLED)
            set_pkr(1, pkr);
    }

    //Enable CR0.WP
    set_wp();
    state->wp = PAGING_FLAG_SET; 

    // Clear RFLAGS.AC 
    unset_rflags_ac();
    state->rflags_ac = PAGING_FLAG_UNSET;

    return state;
}

static inline void ALWAYS_INLINE get_indices(uint64_t addr, uint64_t *pml4e, uint64_t *pdpte, uint64_t *pde, uint64_t *pte)
{
    *pml4e = (addr & PAGING_PML4_INDEX_MASK) >> PAGING_PML4_INDEX_SHIFT;
    *pdpte = (addr & PAGING_PDP_INDEX_MASK) >> PAGING_PDP_INDEX_SHIFT;
    *pde = (addr & PAGING_PD_INDEX_MASK) >> PAGING_PD_INDEX_SHIFT;
    *pte = (addr & PAGING_PT_INDEX_MASK) >> PAGING_PT_INDEX_SHIFT;
}

static inline void* get_pml4e_address(uint64_t pml4e)
{
    return (void *) (0xFFFFFFFFFFFFF000 | (pml4e << 3));
}

static inline void* get_pdpte_address(uint64_t pml4e, uint64_t pdpte)
{
    return (void *) (0xFFFFFFFFFFE00000 | (pml4e << PAGING_PT_INDEX_SHIFT) | (pdpte << 3));
}

static inline void* get_pde_address(uint64_t pml4e, uint64_t pdpte, uint64_t pde)
{
    return (void *) (0xFFFFFFFFC0000000 | (pml4e << PAGING_PD_INDEX_SHIFT) | (pdpte << PAGING_PT_INDEX_SHIFT) | (pde << 3));
}

static inline void* get_pte_address(uint64_t pml4e, uint64_t pdpte, uint64_t pde, uint64_t pte)
{
    return (void *) (0xFFFFFF8000000000 | (pml4e << PAGING_PDP_INDEX_SHIFT) |
        (pdpte << PAGING_PD_INDEX_SHIFT) | (pde << PAGING_PT_INDEX_SHIFT) | (pte << 3));
}

void scratchpad_memory_map(uint64_t virt_addr, uint64_t phys_addr, uint64_t page_count)
{
    for (uint64_t i = 0; i < page_count; i++)
    {   
        uint64_t pml4i, pdpti, pdi, pti;
        get_indices(virt_addr, &pml4i, &pdpti, &pdi, &pti);
        pte *entry = get_pte_address(pml4i, pdpti, pdi, pti);
        memset(entry, 0, 8);
        entry->p = 1;
        entry->r_w = 1;
        entry->address = phys_addr >> 12;
        entry->xd = 1;
        invlpg((void *) virt_addr);
        virt_addr += PAGING_PAGE_SIZE;
        phys_addr += PAGING_PAGE_SIZE;
    }
}

void scratchpad_add_page_table(uint64_t pml4_index, uint64_t pdpt_index, uint64_t pd_index, uint64_t addr)
{
    pde* pde_entry = get_pde_address(pml4_index, pdpt_index, pd_index);
    memset(pde_entry, 0, 8);
    pde_entry->p = 1;
    pde_entry->r_w = 1;
    pde_entry->address = addr >> 12;
    pde_entry->xd = 1;
}

void scratchpad_add_page_directory(uint64_t pml4_index, uint64_t pdpt_index, uint64_t addr)
{
    pdpte *pdpt_entry = get_pdpte_address(pml4_index, pdpt_index);
    memset(pdpt_entry, 0, 8);
    pdpt_entry->p = 1;
    pdpt_entry->r_w = 1;
    pdpt_entry->address = addr >> 12;
    pdpt_entry->xd = 1;
}

void scratchpad_add_page_directory_pointer_table(uint64_t pml4_index, uint64_t addr)
{
    pml4e *pml4_entry = get_pml4e_address(pml4_index);
    memset(pml4_entry, 0, 8);
    pml4_entry->p = 1;
    pml4_entry->r_w = 1;
    pml4_entry->address = addr >> 12;
    pml4_entry->xd = 1;
}

uint64_t get_canonic48(uint64_t addr)
{
    return (addr & ((uint64_t) 1 << 47)) ? (addr | 0xFFFF000000000000) : addr;
}

uint64_t get_page_count(uint64_t size)
{
    return size / PAGING_PAGE_SIZE + (size % PAGING_PAGE_SIZE != 0); 
}