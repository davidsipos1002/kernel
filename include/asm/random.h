#ifndef ASM_RANDOM_H_INCL
#define ASM_RANDOM_H_INCL

#include <stdint.h>

#define RANDOM_SUCCES 1
#define RANDOM_RDRAND_RETRIES 10
#define RANDOM_RDSEED_RETRIES 50

uint8_t random_support();
uint8_t random_rdrand64(uint64_t *rand);
uint8_t random_rdrand64_retry(uint64_t *rand, uint32_t retry);
uint8_t random_rdseed64(uint64_t *rand);
uint8_t random_rdseed64_retry(uint64_t *rand, uint32_t retry);

#endif