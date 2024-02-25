#ifndef GCC_UTILS_H_INCL
#define GCC_UTILS_H_INCL

#define PACKED_STRUCT __attribute__((packed))
#define ALIGN(x) __attribute__((aligned(x)))
#define ALWAYS_INLINE __attribute__((always_inline))
#define INTERRUPT __attribute__((interrupt))

#endif