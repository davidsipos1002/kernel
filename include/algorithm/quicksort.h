#ifndef ALGORITHM_QUICKSORT_H_INCL
#define ALGORITHM_QUICKSORT_H_INCL

#include <stdint.h>

typedef uint8_t (*qs_compar) (const void*, const void*); // 0 - less, 1 - equal, 2 - greater

void quicksort(void *base, uint64_t n, uint64_t size, qs_compar comparator);

#endif