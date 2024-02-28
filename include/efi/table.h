#ifndef EFI_TABLE_H_INCL
#define EFI_TABLE_H_INCL

#include <stdint.h>

//
// Standard EFI table header
//

typedef struct _EFI_TABLE_HEADER {
    uint64_t                      Signature;
    uint32_t                      Revision;
    uint32_t                      HeaderSize;
    uint32_t                      CRC32;
    uint32_t                      Reserved;
} EFI_TABLE_HEADER;

//
// A GUID
//

typedef struct {          
    uint32_t  Data1;
    uint16_t  Data2;
    uint16_t  Data3;
    uint8_t   Data4[8]; 
} EFI_GUID;

//
// EFI Configuration Table and GUID definitions
//

#define MPS_TABLE_GUID    \
    { 0xeb9d2d2f, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }

#define ACPI_TABLE_GUID    \
    { 0xeb9d2d30, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }

#define ACPI_20_TABLE_GUID  \
    { 0x8868e871, 0xe4f1, 0x11d3, {0xbc, 0x22, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} }

#define SMBIOS_TABLE_GUID    \
    { 0xeb9d2d31, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }

#define SMBIOS3_TABLE_GUID    \
    { 0xf2fd1544, 0x9794, 0x4a2c, {0x99, 0x2e, 0xe5, 0xbb, 0xcf, 0x20, 0xe3, 0x94} }

#define SAL_SYSTEM_TABLE_GUID    \
    { 0xeb9d2d32, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }

#define EFI_DTB_TABLE_GUID \
    { 0xb1b621d5, 0xf19c, 0x41a5, {0x83, 0x0b, 0xd9, 0x15, 0x2c, 0x69, 0xaa, 0xe0} }

typedef struct _EFI_CONFIGURATION_TABLE {
    EFI_GUID                VendorGuid;
    void                    *VendorTable;
} EFI_CONFIGURATION_TABLE;

//
// EFI System Table
//

#define EFI_SYSTEM_TABLE_SIGNATURE      0x5453595320494249
#define EFI_1_02_SYSTEM_TABLE_REVISION  EFI_SPECIFICATION_REVISION_MAJORMINOR(1, 02)
#define EFI_1_10_SYSTEM_TABLE_REVISION  EFI_SPECIFICATION_REVISION_MAJORMINOR(1, 10)
#define EFI_2_00_SYSTEM_TABLE_REVISION  EFI_SPECIFICATION_REVISION_MAJORMINOR(2, 00)
#define EFI_2_10_SYSTEM_TABLE_REVISION  EFI_SPECIFICATION_REVISION_MAJORMINOR(2, 10)
#define EFI_2_20_SYSTEM_TABLE_REVISION  EFI_SPECIFICATION_REVISION_MAJORMINOR(2, 20)
#define EFI_2_30_SYSTEM_TABLE_REVISION  EFI_SPECIFICATION_REVISION_MAJORMINOR(2, 30)
#define EFI_2_31_SYSTEM_TABLE_REVISION  EFI_SPECIFICATION_REVISION_MAJORMINOR(2, 31)
#define EFI_2_40_SYSTEM_TABLE_REVISION  EFI_SPECIFICATION_REVISION_MAJORMINOR(2, 40)
#define EFI_2_50_SYSTEM_TABLE_REVISION  EFI_SPECIFICATION_REVISION_MAJORMINOR(2, 50)
#define EFI_2_60_SYSTEM_TABLE_REVISION  EFI_SPECIFICATION_REVISION_MAJORMINOR(2, 60)
#define EFI_2_70_SYSTEM_TABLE_REVISION  EFI_SPECIFICATION_REVISION_MAJORMINOR(2, 70)
#define EFI_2_80_SYSTEM_TABLE_REVISION  EFI_SPECIFICATION_REVISION_MAJORMINOR(2, 80)
#define EFI_2_90_SYSTEM_TABLE_REVISION  EFI_SPECIFICATION_REVISION_MAJORMINOR(2, 90)
#define EFI_2_100_SYSTEM_TABLE_REVISION EFI_SPECIFICATION_REVISION_MAJORMINOR(2, 100)
#define EFI_SYSTEM_TABLE_REVISION       EFI_SPECIFICATION_VERSION

typedef struct _EFI_SYSTEM_TABLE {
    EFI_TABLE_HEADER                Hdr;

    uint16_t                        *FirmwareVendor;
    uint32_t                        FirmwareRevision;

    void                            *ConsoleInHandle;
    void                            *ConIn;

    void                            *ConsoleOutHandle;
    void                            *ConOut;

    void                            *StandardErrorHandle;
    void                            *StdErr;

    void                            *RuntimeServices;
    void                            *BootServices;

    uint64_t                        NumberOfTableEntries;
    EFI_CONFIGURATION_TABLE         *ConfigurationTable;

} EFI_SYSTEM_TABLE;

#endif
