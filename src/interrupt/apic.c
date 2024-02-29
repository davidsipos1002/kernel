#include <interrupt/apic.h>

#include <asm/control.h>
#include <asm/cpuid.h>
#include <gcc/utils.h>

uint8_t apic_get_id()
{
    cpuid_res res;
    cpuid(1, 0, &res); 
    return (uint8_t) ((res.rbx >> 24) & 0xFF);
}

void apic_enable(uint64_t apic_base)
{
    volatile uint32_t *spurious = (volatile uint32_t *) ((volatile uint8_t *) apic_base + APIC_SPURIOUS_INT_REG);
    *spurious |= APIC_SOFTWARE_ENABLE;
}

void apic_disable(uint64_t apic_base)
{
    volatile uint32_t *spurious = (volatile uint32_t *) ((volatile uint8_t *) apic_base + APIC_SPURIOUS_INT_REG);
    *spurious &= ~APIC_SOFTWARE_ENABLE;
}

void apic_mask_all(uint64_t apic_base)
{
    volatile uint8_t *p = (volatile uint8_t *) apic_base;

    volatile uint32_t *reg = (volatile uint32_t *) (p + APIC_LVT_CMCI_REG);
    *reg |= APIC_LVT_MASK; 
    
    reg = (volatile uint32_t *) (p + APIC_LVT_TIMER_REG);
    *reg |= APIC_LVT_TIMER_MASK; 

    reg = (volatile uint32_t *) (p + APIC_LVT_THERMAL_MONITOR_REG);
    *reg |= APIC_LVT_MASK; 
    
    reg = (volatile uint32_t *) (p + APIC_LVT_PERF_COUNTER_REG);
    *reg |= APIC_LVT_MASK; 
    
    reg = (volatile uint32_t *) (p + APIC_LVT_LINT0_REG);
    *reg |= APIC_LVT_MASK; 

    reg = (volatile uint32_t *) (p + APIC_LVT_LINT1_REG);
    *reg |= APIC_LVT_MASK; 

    reg = (volatile uint32_t *) (p + APIC_LVT_ERROR_REG);
    *reg |= APIC_LVT_MASK; 
}

static inline void ALWAYS_INLINE apic_ipi_wait(volatile uint32_t *icrl)
{
    do
        pause();
    while(*icrl & APIC_IPI_STATUS);
}

void apic_send_ipi(uint64_t apic_base, uint8_t destination, uint8_t type, uint8_t vector)
{
    *((volatile uint32_t *) ((volatile uint8_t *) apic_base + APIC_ERROR_STATUS_REG)) = 0;
    volatile uint32_t *icrl = (volatile uint32_t *) ((volatile uint8_t *) apic_base + APIC_IC_L_REG);
    volatile uint32_t *icrh = (volatile uint32_t *) ((volatile uint8_t *) apic_base + APIC_IC_H_REG);
    uint32_t scratch_icrh = 0;
    uint32_t scratch_icrl = 0;

    if (type == APIC_STARTUP_IPI)
        scratch_icrl |= vector;

    scratch_icrh |= (uint32_t) destination << 24;
    scratch_icrl |= (uint32_t) 1 << 14;
    scratch_icrl |= (uint32_t) type << 8;
    
    apic_ipi_wait(icrl);
    *icrh = scratch_icrh;
    *icrl = scratch_icrl;
    apic_ipi_wait(icrl);
}