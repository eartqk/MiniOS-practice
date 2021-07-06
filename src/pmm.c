#include "pmm.h"

#include "common.h"
#include "vmm.h"

uint32_t pmm_stack_loc = PMM_STACK_ADDR;
uint32_t pmm_stack_max = PMM_STACK_ADDR;
uint32_t pmm_location;
char pmm_paging_active = 0;

void init_pmm(uint32_t start) {
	pmm_location = (start + 0x1000) & PAGE_MASK;
}

uint32_t pmm_alloc_page() {
	if (pmm_paging_active) {
		if (pmm_stack_loc == PMM_STACK_ADDR)
			panic("Error:out of memory.");

		pmm_stack_loc -= sizeof(uint32_t);
		uint32_t *stack = (uint32_t *)pmm_stack_loc;

		return *stack;
	} else {
		return pmm_location += 0x1000;
	}
}

void pmm_free_page(uint32_t p) {
	if (p < pmm_location) return;

	if (pmm_stack_max <= pmm_stack_loc) {
		map(pmm_stack_max, p, PAGE_PRESENT | PAGE_WRITE);
		pmm_stack_max += 4096;
	} else {
		uint32_t *stack = (uint32_t *)pmm_stack_loc;
		*stack = p;
		pmm_stack_loc += sizeof(uint32_t);
	}
}