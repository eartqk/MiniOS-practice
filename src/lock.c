#include "lock.h"
#include "common.h"
#include "thread.h"
#include "monitor.h"
#include "scheduler.h"

extern thread_t *current_thread;

static uint32_t atomic_test_and_set (volatile spinlock_t *lock) {
	register spinlock_t value = SPINLOCK_UNLOCKED;
	
	asm volatile("lock					\
								xchgl	%0, %1"
								: "=q" (value), "=m" (*lock)
								: "0" (value));
	
  return value;
}

void spinlock_lock (spinlock_t *lock)
{
  while (atomic_test_and_set (lock) == SPINLOCK_LOCKED);
}

void spinlock_unlock (spinlock_t *lock)
{
  *lock = 0;
}