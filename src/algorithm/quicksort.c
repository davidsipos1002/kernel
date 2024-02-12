#include <algorithm/quicksort.h>

#include <gcc/utils.h>

static inline void ALWAYS_INLINE qs_swap(uint8_t *a, uint8_t *b, uint64_t size)
{
    for (uint64_t i = 0; i < size; i++)
    {
        uint8_t temp = a[i];
        a[i] = b[i];
        b[i] = temp;
    }
}

static uint8_t* partition(uint8_t *start, uint8_t *end, uint64_t size, qs_compar comp)
{
    uint8_t *i = start - size;
    for(uint8_t *p = start;p < end; p += size)
    {
        if (comp(p, end) <= 1)
        {
            i += size;
            qs_swap(p, i, size);
        }
    }
    i += size;
    qs_swap(i, end, size);
    return i;
}

static void qs(uint8_t *start, uint8_t *end, uint64_t size, qs_compar comp)
{
    uint8_t *q = partition(start, end, size, comp);
    if (start < q)
        qs(start, q - size, size, comp);
    if (q + size < end)
        qs(q + size, end, size, comp);
}

void quicksort(void *base, uint64_t n, uint64_t size, qs_compar comparator)
{
    if (n == 1)
        return;
    uint8_t *start = base;
    uint8_t *end = base + (n - 1) * size;
    qs(start, end, size, comparator);
}