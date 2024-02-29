#ifndef INTERRUPT_APIC_H_INCL
#define INTERRUPT_APIC_H_INCL

#include <stdint.h>

#define APIC_SPURIOUS_INT_REG 0xF0
#define APIC_ERROR_STATUS_REG 0x280
#define APIC_LVT_CMCI_REG 0x2F0
#define APIC_IC_L_REG 0x300
#define APIC_IC_H_REG 0x310
#define APIC_LVT_TIMER_REG 0x320
#define APIC_LVT_THERMAL_MONITOR_REG 0x330
#define APIC_LVT_PERF_COUNTER_REG 0x340
#define APIC_LVT_LINT0_REG 0x350
#define APIC_LVT_LINT1_REG 0x360
#define APIC_LVT_ERROR_REG 0x370

#define APIC_SOFTWARE_ENABLE ((uint32_t) 1 << 8)
#define APIC_LVT_TIMER_MASK ((uint32_t) 1 << 16)
#define APIC_LVT_MASK ((uint32_t) 1 << 17)

#define APIC_IPI_STATUS ((uint32_t) 1 << 12)
#define APIC_INIT_IPI 5
#define APIC_STARTUP_IPI 6

uint8_t apic_get_id();
void apic_enable(uint64_t apic_base);
void apic_disable(uint64_t apic_base); 
void apic_mask_all(uint64_t apic_base);
void apic_send_ipi(uint64_t apic_base, uint8_t destination, uint8_t type, uint8_t vector); 
void apic_ipi_wait(uint64_t apic_base);

#endif
