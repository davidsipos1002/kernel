#ifndef EFI_MEMORY_H_INCL
#define EFI_MEMORY_H_INCL

#include <stdint.h>

/*
    Taken from the UEFI specification
*/

typedef enum 
{
   EfiReservedMemoryType,
   EfiLoaderCode,
   EfiLoaderData,
   EfiBootServicesCode,
   EfiBootServicesData,
   EfiRuntimeServicesCode,
   EfiRuntimeServicesData,
   EfiConventionalMemory,
   EfiUnusableMemory,
   EfiACPIReclaimMemory,
   EfiACPIMemoryNVS,
   EfiMemoryMappedIO,
   EfiMemoryMappedIOPortSpace,
   EfiPalCode,
   EfiPersistentMemory,
   EfiUnacceptedMemoryType,
   EfiMaxMemoryType
} EFI_MEMORY_TYPE;

// Memory cacheability attribute
#define EFI_MEMORY_UC            0x0000000000000001
#define EFI_MEMORY_WC            0x0000000000000002
#define EFI_MEMORY_WT            0x0000000000000004
#define EFI_MEMORY_WB            0x0000000000000008
#define EFI_MEMORY_UCE           0x0000000000000010

// Physical memory protection attribute
#define EFI_MEMORY_WP            0x0000000000001000
#define EFI_MEMORY_RP            0x0000000000002000
#define EFI_MEMORY_XP            0x0000000000004000
#define EFI_MEMORY_RO            0x0000000000020000

// Runtime memory attribute
#define EFI_MEMORY_NV            0x0000000000008000
#define EFI_MEMORY_RUNTIME       0x8000000000000000

// Other memory attribute
#define EFI_MEMORY_MORE_RELIABLE 0x0000000000010000
#define EFI_MEMORY_SP            0x0000000000040000
#define EFI_MEMORY_CPU_CRYPTO    0x0000000000080000
#define EFI_MEMORY_ISA_VALID     0x4000000000000000
#define EFI_MEMORY_ISA_MASK      0x0FFFF00000000000

typedef struct 
{
    uint32_t                        Type;           // Field size is 32 bits followed by 32 bit pad
    uint32_t                        Pad;
    uint64_t                        PhysicalStart;  // Field size is 64 bits
    uint64_t                        VirtualStart;   // Field size is 64 bits
    uint64_t                        NumberOfPages;  // Field size is 64 bits
    uint64_t                        Attribute;      // Field size is 64 bits
} EFI_MEMORY_DESCRIPTOR;

#endif