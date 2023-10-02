#include <memory/manipulate.h>

void memset(void *buff, uint8_t val, uint64_t count)
{
    uint8_t *buffer = (uint8_t *) buff;
    for (uint64_t i = 0; i < count; i++)
        buffer[i] = val;
}

void memcpy(void *dest, void *src, uint64_t count)
{
    uint8_t *destination = (uint8_t *) dest;
    uint8_t *source = (uint8_t *) src;
    for (uint64_t i = 0; i < count; i++)
        destination[i] = source[i];
}