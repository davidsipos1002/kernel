#ifndef PAGING_CONTROL_H_INCL
#define PAGING_CONTROL_H_INCL

#include <stdint.h>

/*
    Control paging features
    Unimplemented: 
    1. 5-level paging
    2. control-flow enforcement technology (for shadow-stack)
    3. 1 GB pages
    4. MTRRs (ancient stuff, disable function implemented)
*/

void enable_4_level_paging(uint64_t cr3);
void disable_4_level_paging();

uint8_t enable_global_pages();
uint8_t disable_global_pages();

uint8_t is_pat_supported();

uint8_t enable_pcid();
uint8_t disable_pcid();

uint8_t enable_smep();
uint8_t disable_smep();

uint8_t enable_smap();
uint8_t disable_smap();

uint8_t enable_pku();
uint8_t disable_pku();

uint8_t enable_pks();
uint8_t disable_pks();

uint8_t enable_nx();
uint8_t disable_nx();

void get_address_widths(uint8_t *physical, uint8_t *linear);

uint8_t disable_mtrr();

uint64_t get_pat();
void set_pat(uint64_t pat);
void format_pat_entry(uint64_t *pat, uint8_t entry, uint8_t type);

uint64_t get_pkr(uint8_t supervisor);
void set_pkr(uint8_t supervisor, uint64_t pkr);
void format_pkr(uint64_t *pkr, uint8_t key, uint8_t write_disable, uint8_t access_disable); 

void set_rflags_ac();
void unset_rflags_ac();

void set_wp();
void unset_wp();

#endif