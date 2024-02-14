#ifndef SEGMENT_FORMAT_H_INCL
#define SEGMENT_FORMAT_H_INCL

#include <stdint.h>

#include <gcc/utils.h>

typedef struct
{
   uint64_t limit_0 : 16;
   uint64_t base_0 : 16;
   uint64_t base_1 : 8;
   uint64_t type : 4;
   uint64_t s : 1;
   uint64_t dpl : 2;
   uint64_t p : 1;
   uint64_t limit_1 : 4;
   uint64_t avl : 1;
   uint64_t l : 1;
   uint64_t db : 1;
   uint64_t g : 1;
   uint64_t base_2 : 8;
} PACKED_STRUCT segment_descriptor;

typedef struct
{
   uint64_t limit_0 : 16;
   uint64_t base_0 : 16;
   uint64_t base_1 : 8;
   uint64_t type : 4;
   uint64_t s : 1;
   uint64_t dpl : 2;
   uint64_t p : 1;
   uint64_t limit_1 : 4;
   uint64_t avl : 1;
   uint64_t l : 1;
   uint64_t db : 1;
   uint64_t g : 1;
   uint64_t base_2 : 8;
   uint64_t base_3 : 32;
   uint64_t reserved : 32;
} PACKED_STRUCT ldt64_descriptor;

#endif