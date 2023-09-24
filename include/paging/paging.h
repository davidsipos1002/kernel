#ifndef PAGING_PAGING_H_INCL
#define PAGING_PAGING_H_INCL

#include <stdint.h>
#include <paging/state.h>

paging_state* paging_init(uint64_t cr3, void *state_location);

#endif