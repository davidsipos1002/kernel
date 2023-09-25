#include <memory/manipulate.h>

void memset(void *buff, uint8_t val, uint64_t count)
{
    uint8_t *buffer = (uint8_t *) buff;
    for (uint64_t i = 0; i < count; i++)
        buffer[i] = val;
}