#ifndef PAGING_PAGING_H_INCL
#define PAGING_PAGING_H_INCL

#include <stdint.h>
#include <paging/state.h>

paging_state* paging_init(uint64_t cr3, void *state_location);
uint8_t scratchpad_memory_map(uint64_t addr, uint64_t page_count);

uint64_t get_canonic48(uint64_t addr);
uint64_t get_page_count(uint64_t size);

#endif