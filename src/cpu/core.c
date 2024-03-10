#include <cpu/core.h>

#include <acpi/table.h>
#include <asm/io.h>
#include <asm/registers.h>
#include <efi/table.h>
#include <interrupt/apic.h>
#include <interrupt/pic.h>
#include <memory/manipulate.h>
#include <paging/paging.h>
#include <segment/format.h>

typedef struct
{
    uint32_t cr3;
    uint64_t rsp;
    uint32_t vector;
    uint64_t ap_func;
    uint64_t param;
    uint8_t on;
} PACKED_STRUCT ap_asm_s;

extern char __ap_init_begin[];
extern char __ap_init_end[];
extern char __ap_init_ljmp_instr[];
extern char __ap_init_ljmp[];
extern char __ap_init_gdt[];
extern char __ap_init_gdtr[];
extern char __ap_init_protected[];
extern char __ap_init_absljmp_instr[];
extern char __ap_init_absljmp[];
extern char __ap_init_ljmp64_instr[];
extern char __ap_init_ljmp64[];

void parse_efi_system_table(BootInfo *bootInfo, uint8_t *core_ids, uint8_t *count, uint64_t *apic_addr)
{
    uint64_t addr = PAGING_ALIGN(bootInfo->efi_system_table);
    scratchpad_memory_map(0, addr, 5, 0, 1);
    EFI_SYSTEM_TABLE *st = (EFI_SYSTEM_TABLE *) PAGING_PAGE_OFFSET(bootInfo->efi_system_table); 
    EFI_CONFIGURATION_TABLE *cfg = (EFI_CONFIGURATION_TABLE *) PAGING_PAGE_OFFSET(st->ConfigurationTable);
    EFI_GUID acpi_guid = ACPI_TABLE_GUID;
    uint64_t root_sys_desc = 0;
    for (uint64_t i = 0; i < st->NumberOfTableEntries && !root_sys_desc; i++)
    {
        if (memeq(&acpi_guid, &cfg->VendorGuid, sizeof(EFI_GUID)))
            root_sys_desc = (uint64_t) cfg->VendorTable;
        cfg++;
    }

    if (root_sys_desc)
    {
        addr = PAGING_ALIGN(root_sys_desc);
        scratchpad_memory_map(0, addr, 1, 0, 1);
        acpi_rsdp *rsdp = (acpi_rsdp *) PAGING_PAGE_OFFSET(root_sys_desc);
        if (acpi_validate_rsdp(rsdp))
        {
            addr = PAGING_ALIGN(rsdp->rsdtaddress);
            acpi_rsdt *rsdt = (acpi_rsdt *) PAGING_PAGE_OFFSET(rsdp->rsdtaddress);
            scratchpad_memory_map(0, addr, 1, 0, 1);
            if (acpi_validate_rsdt(rsdt))
            {
                uint32_t length;
                uint64_t addr = acpi_1_get_table_pointer(rsdt, ACPI_SIGNATURE_MADT, &length);
                if (addr)
                {
                    uint64_t page_count = get_page_count(length);
                    scratchpad_memory_map(0, PAGING_ALIGN(addr), page_count, 0, 1);
                    acpi_madt *madt = (acpi_madt *) PAGING_PAGE_OFFSET(addr);
                    acpi_madt_get_processors(madt, core_ids, count, apic_addr); 
                }
            }
        }
    }
}

uint8_t place_ap_init_code(mem_map *map)
{
    const uint64_t mask = 0xFF000;
    uint64_t addr = 0;
    for (uint64_t i = 0; i < map->length; i++)
    {
        mem_region *reg = &map->map[i];
        if (reg->start < 0x100000)
        {
            addr = reg->start;
            break;
        }
    }
    if (!addr)
        return 0; 
    uint64_t size = (uint8_t *) __ap_init_end - (uint8_t *) __ap_init_begin;
    scratchpad_memory_map(0, addr, 1, 0, 1);
    memset(0, 0, PAGING_PAGE_SIZE);
    memcpy(0, (void *) __ap_init_begin, size);

    uint32_t *gdt_base = (uint32_t *) (__ap_init_gdtr - __ap_init_begin + 2);
    *gdt_base = addr + __ap_init_gdt - __ap_init_begin;
    
    uint16_t *ljmp = (uint16_t *) (__ap_init_ljmp_instr - __ap_init_begin + 1);
    *ljmp = __ap_init_ljmp - __ap_init_begin;
    
    uint32_t *absljmp = (uint32_t *) (__ap_init_absljmp_instr - __ap_init_begin + 1);
    *absljmp = addr + __ap_init_absljmp - __ap_init_begin;
    
    uint32_t *ljmp64 = (uint32_t *) (__ap_init_ljmp64_instr - __ap_init_begin + 1);
    *ljmp64 = addr + __ap_init_ljmp64 - __ap_init_begin;

    return (uint8_t) ((addr & mask) >> PAGING_PAGE_SIZE_EXP);
}

uint8_t start_ap_cores(uint8_t ap_vector, uint64_t apic_addr, ap_init *init, uint8_t count)
{
    scratchpad_memory_map(0x110000, apic_addr, 1, 3, 1);
    pic_disable();

    uint8_t apic_id = apic_get_id();
    apic_mask_all(0x110000);
    apic_enable(0x110000);

    uint64_t size = __ap_init_end - __ap_init_begin;
    uint64_t ap_init_code = (uint64_t) ap_vector << PAGING_PAGE_SIZE_EXP;
    scratchpad_memory_map(ap_init_code, ap_init_code, 1, 0, 0);
    scratchpad_memory_map(0x111000, ap_init_code, 1, 0, 1);
    volatile ap_asm_s *ap_asm = (volatile ap_asm_s *) (0x111000 + size - sizeof(ap_asm_s));
    ap_asm->cr3 = get_cr3();
    ap_asm->vector = (uint32_t) ap_vector << PAGING_PAGE_SIZE_EXP;
    uint8_t started = 0;

    for (uint8_t i = 0; i < count; i++)
    {
        if (init[i].core_id == apic_id)
            continue;
        segment_descriptor *desc = (segment_descriptor *) (__ap_init_gdt - __ap_init_begin);
        desc++;
        desc->base_0 = ap_init_code & 0xFFFF;
        desc->base_1 = (ap_init_code >> 16) & 0xF;
        desc++;
        desc->base_0 = ap_init_code & 0xFFFF;
        desc->base_1 = (ap_init_code >> 16) & 0xF;
        ap_asm->rsp = (uint64_t) init[i].stack;
        ap_asm->ap_func = (uint64_t) init[i].start;
        ap_asm->param = (uint64_t) init[i].param;
        ap_asm->on = 0;
        apic_send_ipi(0x110000, init[i].core_id, APIC_INIT_IPI, 0);
        apic_ipi_wait(0x110000);
        for (uint16_t j = 0; j < 10000; j++)
            io_wait();
        apic_send_ipi(0x110000, init[i].core_id, APIC_STARTUP_IPI, ap_vector);
        apic_ipi_wait(0x110000);
        for (uint16_t j = 0; j < 10000; j++)
            io_wait();
        if (!ap_asm->on) {
            apic_send_ipi(0x110000, init[i].core_id, APIC_STARTUP_IPI, ap_vector);
            apic_ipi_wait(0x110000);
            for (uint16_t j = 0; j < 10000; j++)
                io_wait();
        }
        if (ap_asm->on)
            started++;
    }
    
    return started;
}

 
