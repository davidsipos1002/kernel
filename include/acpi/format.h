#ifndef ACPI_FORMAT_H_INCL
#define ACPI_FORMAT_H_INCL

#include <stdint.h>

#include <gcc/utils.h>

#define ACPI_1_REVISION 0
#define ACPI_2_REVISION 2

#define ACPI_SIGNATURE_RSDP "RSD PTR "
#define ACPI_SIGNATURE_RSDT "RSDT"
#define ACPI_SIGNATURE_MADT "APIC"

#define ACPI_MADT_PROCESSOR_LOCAL_APIC 0
#define ACPI_MADT_PROCESSOR_LOCAL_APIC_OVERRIDE 5

#define ACPI_APIC_LOCAL_FLG_ENABLED 0x1
#define ACPI_APIC_LOCAL_FLG_ONLINE_CAPABLE 0x2

typedef struct
{
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oemid[6];
    uint8_t oemtableid[8];
    uint32_t oemrevision;
    uint32_t creatorid;
    uint32_t creatorevision;
} PACKED_STRUCT acpi_header;

typedef struct
{
    char signature[8];
    uint8_t checksum;
    uint8_t oemid[6];
    uint8_t revision;
    uint32_t rsdtaddress;
    uint32_t length;
    uint64_t xsdtaddress;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} PACKED_STRUCT acpi_rsdp;

typedef struct 
{
    acpi_header header;
    uint32_t entry;
} PACKED_STRUCT acpi_rsdt;

typedef struct
{
    acpi_header header;
    uint32_t apic_addr;
    uint32_t flags;
} PACKED_STRUCT acpi_madt;

typedef struct
{
    uint8_t type;
    uint8_t length;
} PACKED_STRUCT acpi_interrupt_header;

typedef struct
{
    acpi_interrupt_header header;
    uint8_t acpi_uid;
    uint8_t apic_id;
    uint32_t flags;
} PACKED_STRUCT acpi_processor_apic;

typedef struct
{
    acpi_interrupt_header header;
    uint16_t reserved;
    uint64_t apic_addr;
} PACKED_STRUCT acpi_apic_addr_override;

#endif
