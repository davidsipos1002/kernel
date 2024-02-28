#include <acpi/table.h>

#include <memory/manipulate.h>
#include <paging/paging.h>

static uint8_t acpi_checksum(uint8_t *p, uint64_t length)
{
    uint8_t sum = 0;
    for (uint32_t i = 0; i < length; i++)
        sum += ((uint8_t *) p)[i];
    return !sum;
}

uint8_t acpi_validate_rsdp(acpi_rsdp *rsdp)
{
    if (!acpi_checksum((uint8_t *) rsdp, 20))
        return 0;
    if (!memeq(rsdp->signature, ACPI_SIGNATURE_RSDP, 8))
        return 0;
    return 1;
}

uint8_t acpi_validate_rsdt(acpi_rsdt *rsdt)
{
    if (!memeq(rsdt->header.signature, ACPI_SIGNATURE_RSDT, 4))
        return 0;
    if (!acpi_checksum((uint8_t *) rsdt, rsdt->header.length))
        return 0;
    return 1;
}

uint64_t acpi_1_get_table_pointer(acpi_rsdt *rsdt, const char *signature, uint32_t *length)
{
    uint32_t count = (rsdt->header.length - sizeof(acpi_header)) / 4;
    uint32_t *ptrs = &rsdt->entry;
    for (uint32_t i = 0; i < count; i++)
    {
        uint32_t addr = ptrs[i];
        scratchpad_memory_map(0x1000, PAGING_ALIGN(addr), 1);
        acpi_header *header = (acpi_header *) (0x1000 + PAGING_PAGE_OFFSET(addr));
        if (memeq(header->signature, signature, 4)) {
            *length = header->length; 
            uint64_t page_count = get_page_count(*length);
            if (page_count > 1)
                scratchpad_memory_map(0x1000, PAGING_ALIGN(addr), page_count);
            if (acpi_checksum((uint8_t *) header, *length))
                return addr;
        }
    }
    return 0;
}

void acpi_madt_get_processors(acpi_madt *madt, uint8_t *apic_ids, uint8_t *count, uint64_t *apic_addr)
{
    int32_t length = madt->header.length - sizeof(acpi_madt);
    uint8_t *ptr = (uint8_t *) madt + sizeof(acpi_madt);
    uint8_t *end = (uint8_t *) ptr + length;
    uint8_t i = 0;
    *apic_addr = madt->apic_addr;
    while (ptr < end)
    {
        acpi_interrupt_header *header = (acpi_interrupt_header *) ptr;
        if (header->type == ACPI_MADT_PROCESSOR_LOCAL_APIC)
        {
            acpi_processor_apic *core = (acpi_processor_apic *) ptr;
            if ((core->flags & ACPI_APIC_LOCAL_FLG_ENABLED) || (core->flags & ACPI_APIC_LOCAL_FLG_ONLINE_CAPABLE))
                apic_ids[i++] = core->apic_id;
        }
        else if (header->type == ACPI_MADT_PROCESSOR_LOCAL_APIC_OVERRIDE)
        {
            acpi_apic_addr_override *override = (acpi_apic_addr_override *) ptr;
            *apic_addr = override->apic_addr;
        }
        ptr += header->length;
    }
    *count = i;
} 