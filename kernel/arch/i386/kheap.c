#include <kernel/kheap.h>
#include <kernel/paging.h>
#include <kernel/pmm.h>

static uint32_t heap_start;
static uint32_t heap_end;
static uint32_t heap_next;

void kheap_initialize(uint32_t start, uint32_t size) {
	heap_start = heap_next = start;
	heap_end   = start + size;

	/* Back the whole range with physical frames */
	for (uint32_t a = start; a < heap_end; a += 0x1000)
		paging_map(a, pmm_alloc_frame(), PAGE_WRITE);
}

void* kmalloc(size_t size) {
	if (heap_next + size > heap_end)
		return NULL;                    /* heap exhausted */

	uint32_t addr = heap_next;
	heap_next += (size + 7) & ~7U;      /* keep 8-byte alignment */
	return (void*) addr;
}

void* kmalloc_aligned(size_t size) {
	heap_next = (heap_next + 0xFFF) & ~0xFFFU;
	return kmalloc(size);
}

uint32_t kheap_used(void) {
	return heap_next - heap_start;
}
