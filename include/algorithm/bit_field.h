#ifndef ALGORTIHM_BIT_FIELD_H_INCL
#define ALGORTIHM_BIT_FIELD_H_INCL

#include <stdint.h>

#define bit_field_set_bit(bitmap, bit_ndx) ((uint8_t *) bitmap)[((uint64_t) bit_ndx) >> 3] |= (1 << (bit_ndx & 0x7))
#define bit_field_unset_bit(bitmap, bit_ndx) ((uint8_t *) bitmap)[((uint64_t) bit_ndx) >> 3] &= ~(1 << (bit_ndx & 0x7))
#define bit_field_toggle_bit(bitmap, bit_ndx) ((uint8_t *) bitmap)[((uint64_t) bit_ndx) >> 3] ^= (1 << (bit_ndx & 0x7))
#define bit_field_get_bit(bitmap, bit_ndx) ((((uint8_t *) bitmap)[((uint64_t) bit_ndx) >> 3] & (1 << (bit_ndx & 0x7))) != 0)

#endif