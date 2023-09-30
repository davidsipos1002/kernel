#include <asm/random.h>

#include <asm/cpuid.h>
#include <cpu/features.h>

uint8_t random_support()
{
    cpuid_res res;
    cpuid(1, 0, &res);
    if (!(res.rcx & RANDOM_RAX_01_RCX_RDRAND))
        return 0;
    cpuid(7, 0, &res);
    if (!(res.rbx & RANDOM_RAX_07_RCX_0_RBX_RDSEED))
        return 0;
    return 1;
}

uint8_t random_rdrand64(uint64_t *rand)
{
    uint8_t ok = 0;
    __asm__ __volatile__ ("rdrand %0; setc %1"
        : "=r" (*rand), "=m" (ok)
        :
        : "memory");
    return ok;
}

uint8_t random_rdrand64_retry(uint64_t *rand, uint32_t retry)
{
    for (uint32_t i = 0; i < retry; i++)
        if (random_rdrand64(rand))
            return 1;
    return 0;
}

uint8_t random_rdseed64(uint64_t *rand)
{
    uint8_t ok = 0;
    __asm__ __volatile__ ("rdseed %0; setc %1"
        : "=r" (*rand), "=m" (ok)
        :
        : "memory");
    return ok;
}

uint8_t random_rdseed64_retry(uint64_t *rand, uint32_t retry)
{
    for (uint32_t i = 0; i < retry; i++)
        if (random_rdseed64(rand))
            return 1;
    return 0;
}