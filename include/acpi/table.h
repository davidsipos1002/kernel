#ifndef ACPI_TABLE_H_INCL
#define ACPI_TABLE_H_INCL

#include <stdint.h>

#include <acpi/format.h>

uint8_t acpi_validate_rsdp(acpi_rsdp *rdsp);
uint8_t acpi_validate_rsdt(acpi_rsdt *rsdt);
uint64_t acpi_1_get_table_pointer(acpi_rsdt *rsdt, const char *signature, uint32_t *length);
void acpi_madt_get_processors(acpi_madt *madt, uint8_t *apic_ids, uint8_t *count, uint64_t *apic_addr);

#endif
