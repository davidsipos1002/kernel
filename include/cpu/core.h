#ifndef CPU_CORE_H_INCL
#define CPU_CORE_H_INCL

#include <stdint.h>

#include <boot/bootinfo.h>
#include <memory/memory_map.h>
#include <sync/spinlock.h>

typedef void (*ap_main) (void*);

typedef struct
{
    uint8_t core_id;
    void *stack;
    ap_main start;
    void *param;
} ap_init;

void parse_efi_system_table(BootInfo *bootInfo, uint8_t *core_ids, uint8_t *count, uint64_t *apic_addr); 
uint8_t place_ap_init_code(mem_map *map);
uint8_t start_ap_cores(uint8_t ap_vector, uint64_t apic_addr, ap_init *init, uint8_t count);

#endif
