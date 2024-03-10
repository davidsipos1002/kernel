#include <sync/spinlock.h>

extern void __spinlock_get(void *lock);
extern void __spinlock_release(void *lock);

void spinlock_lock(spinlock *lock)
{
    __spinlock_get(lock);
}

void spinlock_release(spinlock *lock)
{
    __spinlock_release(lock);
}