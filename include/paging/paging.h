#ifndef PAGING_PAGING_H_INCL
#define PAGING_PAGING_H_INCL

#include <stdint.h>
#include <paging/state.h>

#define PAGING_PAGE_TABLE_LENGTH 512

paging_state* paging_init(uint64_t cr3, void *state_location);

void scratchpad_memory_map(uint64_t virt_addr, uint64_t phys_addr, uint64_t page_count);
void scratchpad_add_page_table(uint64_t pml4_index, uint64_t pdpt_index, uint64_t pd_index, uint64_t addr);
void scratchpad_add_page_directory(uint64_t pml4_index, uint64_t pdpt_index, uint64_t addr);
void scratchpad_add_page_directory_pointer_table(uint64_t pml4_index, uint64_t addr);

uint64_t get_canonic48(uint64_t addr);
uint64_t get_page_count(uint64_t size);

uint64_t paging_get_virtual_address(uint64_t pml4_index, uint64_t pdpt_index, uint64_t pd_index, uint64_t pt_index);
uint64_t paging_translate(uint64_t vaddr);

#endif