#include "vmm.h"

#include "idt.h"
#include "pmm.h"

u32int *page_directory = (u32int *)PAGE_DIR_VIRTUAL_ADDR;
u32int *page_tables = (u32int *)PAGE_TABLE_VIRTUAL_ADDR;

page_directory_t *current_directory;

extern char pmm_paging_active;

void page_fault(registers_t *regs);

void init_vmm() {
	int i;
	u32int cr0;

	// Register the page fault handler.
	register_interrupt_handler(14, &page_fault);

	// Create a page directory.
	page_directory_t *pd = (page_directory_t *)pmm_alloc_page();

	// Initialise it.
	memset(pd, 0, 0x1000);

	// Identity map the first 4 MB.
	pd[0] = pmm_alloc_page() | PAGE_PRESENT | PAGE_WRITE;
	u32int *pt = (u32int *)(pd[0] & PAGE_MASK);
	for (i = 0; i < 1024; i++)
		pt[i] = i * 0x1000 | PAGE_PRESENT | PAGE_WRITE;

	// Assign the second-last table and zero it.
	pd[1022] = pmm_alloc_page() | PAGE_PRESENT | PAGE_WRITE;
	pt = (u32int *)(pd[1022] & PAGE_MASK);
	memset(pt, 0, 0x1000);

	// The last entry of the second-last table is the directory itself.
	pt[1023] = (u32int)pd | PAGE_PRESENT | PAGE_WRITE;

	// The last table loops back on the directory itself.
	pd[1023] = (u32int)pd | PAGE_PRESENT | PAGE_WRITE;

	// Set the current directory.
	switch_page_directory(pd);

	// Enable paging.
	asm volatile("mov %%cr0, %0"
	             : "=r"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0"
	             :
	             : "r"(cr0));

	// We need to map the page table where the physical memory manager keeps its page stack
	// else it will panic on the first "pmm_free_page".
	u32int pt_idx = PAGE_DIR_IDX((PMM_STACK_ADDR >> 12));
	page_directory[pt_idx] = pmm_alloc_page() | PAGE_PRESENT | PAGE_WRITE;
	memset(page_tables[pt_idx * 1024], 0, 0x1000);

	// Paging is now active. Tell the physical memory manager.
	pmm_paging_active = 1;
}

void switch_page_directory(page_directory_t *pd) {
	current_directory = pd;
	asm volatile("mov %0, %%cr3"
	             :
	             : "r"(pd));
}

void map(u32int va, u32int pa, u32int flags) {
	u32int virtual_page = va / 0x1000;
	u32int pt_idx = PAGE_DIR_IDX(virtual_page);

	// Find the appropriate page table for 'va'.
	if (page_directory[pt_idx] == 0) {
		// The page table holding this page has not been created yet.
		page_directory[pt_idx] = pmm_alloc_page() | PAGE_PRESENT | PAGE_WRITE;
		memset(page_tables[pt_idx * 1024], 0, 0x1000);
	}

	// Now that the page table definately exists, we can update the PTE.
	page_tables[virtual_page] = (pa & PAGE_MASK) | flags;
}

void unmap(u32int va) {
	u32int virtual_page = va / 0x1000;

	page_tables[virtual_page] = 0;
	// Inform the CPU that we have invalidated a page mapping.
	asm volatile("invlpg (%0)"
	             :
	             : "a"(va));
}

char get_mapping(u32int va, u32int *pa) {
	u32int virtual_page = va / 0x1000;
	u32int pt_idx = PAGE_DIR_IDX(virtual_page);

	// Find the appropriate page table for 'va'.
	if (page_directory[pt_idx] == 0)
		return 0;

	if (page_tables[virtual_page] != 0) {
		if (pa) *pa = page_tables[virtual_page] & PAGE_MASK;
		return 1;
	}
}

void page_fault(registers_t *regs) {
	u32int cr2;
	asm volatile("mov %%cr2, %0"
	             : "=r"(cr2));

	printk("Page fault at 0x%x, faulting address 0x%x\n", regs->eip, cr2);
	printk("Error code: %x\n", regs->err_code);
	panic("");
	for (;;)
		;
}