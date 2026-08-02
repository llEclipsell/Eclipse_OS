#include <string.h>
#include <kernel/paging.h>
#include <kernel/pmm.h>

static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t first_table[1024]    __attribute__((aligned(4096)));

extern void paging_load(uint32_t page_dir_phys);

void paging_initialize(void) {
	/* Every directory entry absent, but writable for later use */
	for (int i = 0; i < 1024; i++)
		page_directory[i] = PAGE_WRITE;

	/* Identity-map the first 4 MiB */
	for (int i = 0; i < 1024; i++)
		first_table[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_WRITE;

	page_directory[0] = ((uint32_t) first_table) | PAGE_PRESENT | PAGE_WRITE;

	paging_load((uint32_t) page_directory);
}
