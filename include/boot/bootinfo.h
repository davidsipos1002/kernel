#ifndef BOOTINFO_H_INCL
#define BOOTINFO_H_INCL

#include <stdint.h>

typedef struct 
{
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t pixel_size;
    uint64_t base;
} FrameBuffer;

typedef struct 
{
    uint64_t descriptor_size;
    uint64_t size;
    uint64_t map;
} MemoryMap;

typedef struct 
{
    uint64_t efi_system_table;
    FrameBuffer framebuffer;
    MemoryMap memorymap; 
} BootInfo;

#endif