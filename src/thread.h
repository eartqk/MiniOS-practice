#ifndef THREAD_H
#define THREAD_H

#include "common.h"

typedef struct
{
	u32int esp, ebp, ebx, esi, edi, eflags;
	u32int id;  // Thread ID.
} thread_t;

thread_t *init_threading();

thread_t *create_thread(int (*fn)(void *), void *arg, u32int *stack);

struct thread_list;

void switch_thread(struct thread_list *next);

#endif