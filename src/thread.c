#include "thread.h"

thread_t *current_thread;
u32int next_tid = 0;

void thread_exit();

extern void _create_thread(int (*)(void *), void *, u32int *, thread_t *);

thread_t *init_threading() {
	thread_t *thread = kmalloc(sizeof(thread_t));
	thread->id = next_tid++;

	current_thread = thread;

	return thread;
}

thread_t *create_thread(int (*fn)(void *), void *arg, u32int *stack) {
	thread_t *thread = kmalloc(sizeof(thread_t));
	memset(thread, 0, sizeof(thread_t));
	thread->id = next_tid++;

	*--stack = (u32int)arg;
	*--stack = (u32int)&thread_exit;
	*--stack = (u32int)fn;

	thread->esp = (u32int)stack;
	thread->ebp = 0;
	thread->eflags = 0x200;  // Interrupts enabled.
	thread_is_ready(thread);

	return thread;
}

void thread_exit() {
	register u32int val asm("eax");

	printk("Thread exited with value %d\n", val);

	for (;;)
		;
}