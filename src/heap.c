#include "heap.h"

#include "pmm.h"
#include "vmm.h"

static void alloc_chunk(u32int start, u32int len);
static void free_chunk(header_t *chunk);
static void split_chunk(header_t *chunk, u32int len);
static void glue_chunk(header_t *chunk);

u32int heap_max = HEAP_START;
header_t *heap_first = 0;

void init_heap() {
}

void *kmalloc(u32int l) {
	l += sizeof(header_t);

	header_t *cur_header = heap_first, *prev_header = 0;
	while (cur_header) {
		if (cur_header->allocated == 0 && cur_header->length >= l) {
			split_chunk(cur_header, l);
			cur_header->allocated = 1;
			return (void *)((u32int)cur_header + sizeof(header_t));
		}
		prev_header = cur_header;
		cur_header = cur_header->next;
	}

	u32int chunk_start;
	if (prev_header)
		chunk_start = (u32int)prev_header + prev_header->length;
	else {
		chunk_start = HEAP_START;
		heap_first = (header_t *)chunk_start;
	}

	alloc_chunk(chunk_start, l);
	cur_header = (header_t *)chunk_start;
	cur_header->prev = prev_header;
	cur_header->next = 0;
	cur_header->allocated = 1;
	cur_header->length = l;

	prev_header->next = cur_header;

	return (void *)(chunk_start + sizeof(header_t));
}

void kfree(void *p) {
	header_t *header = (header_t *)((u32int)p - sizeof(header_t));
	header->allocated = 0;

	glue_chunk(header);
}

void alloc_chunk(u32int start, u32int len) {
	while (start + len > heap_max) {
		u32int page = pmm_alloc_page();
		map(heap_max, page, PAGE_PRESENT | PAGE_WRITE);
		heap_max += 0x1000;
	}
}

void free_chunk(header_t *chunk) {
	chunk->prev->next = 0;

	if (chunk->prev == 0)
		heap_first = 0;

	// While the heap max can contract by a page and still be greater than the chunk address...
	while ((heap_max - 0x1000) >= (u32int)chunk) {
		heap_max -= 0x1000;
		u32int page;
		get_mapping(heap_max, &page);
		pmm_free_page(page);
		unmap(heap_max);
	}
}

void split_chunk(header_t *chunk, u32int len) {
	// In order to split a chunk, once we split we need to know that there will be enough
	// space in the new chunk to store the chunk header, otherwise it just isn't worthwhile.
	if (chunk->length - len > sizeof(header_t)) {
		header_t *newchunk = (header_t *)((u32int)chunk + chunk->length);
		newchunk->prev = chunk;
		newchunk->next = 0;
		newchunk->allocated = 0;
		newchunk->length = chunk->length - len;

		chunk->next = newchunk;
		chunk->length = len;
	}
}

void glue_chunk(header_t *chunk) {
	if (chunk->next && chunk->next->allocated == 0) {
		chunk->length = chunk->length + chunk->next->length;
		chunk->next->next->prev = chunk;
		chunk->next = chunk->next->next;
	}

	if (chunk->prev && chunk->prev->allocated == 0) {
		chunk->prev->length = chunk->prev->length + chunk->length;
		chunk->prev->next = chunk->next;
		chunk->next->prev = chunk->prev;
		chunk = chunk->prev;
	}

	if (chunk->next == 0)
		free_chunk(chunk);
}