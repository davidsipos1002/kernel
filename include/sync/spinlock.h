#ifndef SYNC_SPINLOCK_H_INCL
#define SYNC_SPINLOCK_H_INCL

#include <stdint.h>

#include <gcc/utils.h>

typedef struct
{
    uint8_t lock[128];
} spinlock;

void spinlock_lock(spinlock *lock);
void spinlock_release(spinlock *lock);

#endif