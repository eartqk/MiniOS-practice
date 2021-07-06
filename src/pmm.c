#include "pmm.h"

#include "common.h"
#include "vmm.h"

u32int pmm_stack_loc = PMM_STACK_ADDR;
u32int pmm_stack_max = PMM_STACK_ADDR;
u32int pmm_location;
char pmm_paging_active = 0;

void init_pmm(u32int start) {
	pmm_location = (start + 0x1000) & PAGE_MASK;
}

u32int pmm_alloc_page() {
	if (pmm_paging_active) {
		if (pmm_stack_loc == PMM_STACK_ADDR)
			panic("Error:out of memory.");

		pmm_stack_loc -= sizeof(u32int);
		u32int *stack = (u32int *)pmm_stack_loc;

		return *stack;
	} else {
		return pmm_location += 0x1000;
	}
}

void pmm_free_page(u32int p) {
	if (p < pmm_location) return;

	if (pmm_stack_max <= pmm_stack_loc) {
		map(pmm_stack_max, p, PAGE_PRESENT | PAGE_WRITE);
		pmm_stack_max += 4096;
	} else {
		u32int *stack = (u32int *)pmm_stack_loc;
		*stack = p;
		pmm_stack_loc += sizeof(u32int);
	}
}