#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include "tas.h"

typedef struct spinlock {
        volatile char p_lock;
}spinlock;

void spin_lock(struct spinlock *lock_arg);

void spin_unlock(struct spinlock *lock_arg);

#endif
