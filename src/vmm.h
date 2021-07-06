#ifndef VMM_H
#define VMM_H

#include "common.h"

#define PAGE_DIR_VIRTUAL_ADDR 0xFFBFF000
#define PAGE_TABLE_VIRTUAL_ADDR 0xFFC00000
#define PAGE_DIR_IDX(x) ((u32int)x / 1024)
#define PAGE_TABLE_IDX(x) ((u32int)x % 1024)

#define PAGE_PRESENT 0x1
#define PAGE_WRITE 0x2
#define PAGE_USER 0x4
#define PAGE_MASK 0xFFFFF000

typedef u32int page_directory_t;

// Sets up the environment, page directories etc and enables paging.
void init_vmm();

// Changes address space.
void switch_page_directory(page_directory_t *pd);

// Maps the physical page "pa" into the virtual space at address "va", using
// page protection flags "flags".
void map(u32int va, u32int pa, u32int flags);

// Removes one page of V->P mappings at virtual address "va".
void unmap(u32int va);

// Returns 1 if the given virtual address is mapped in the address space.
// If "*pa" is non-NULL, the physical address of the mapping is placed in *pa.
char get_mapping(u32int va, u32int *pa);

#endif