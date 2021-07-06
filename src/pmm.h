#ifndef PMM_H
#define PMM_H

#include "common.h"

#define PMM_STACK_ADDR 0xFF000000

void init_pmm(u32int start);

u32int pmm_alloc_page();

void pmm_free_page(u32int p);

#endif