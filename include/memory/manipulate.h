#ifndef MEMORY_MANIPULATE_H_INCL
#define MEMORY_MANIPULATE_H_INCL

#include <stdint.h>

void memset(void *buff, uint8_t val, uint64_t count);
void memcpy(void *dest, void *src, uint64_t count);
uint8_t memeq(const void *a, const void *b, uint64_t size);

#endif