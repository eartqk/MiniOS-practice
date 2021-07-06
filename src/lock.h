#ifndef LOCK_H
#define LOCK_H

#include "common.h"
#include "scheduler.h"
#include "thread.h"

#define SPINLOCK_LOCKED 0
#define SPINLOCK_UNLOCKED 1

typedef volatile u32int spinlock_t;

typedef struct semaphore {
	u32int counter;
	spinlock_t lock;
	thread_list_t queue;
} semaphore_t;

void spinlock_lock(spinlock_t *lock);

void spinlock_unlock(spinlock_t *lock);

#endif