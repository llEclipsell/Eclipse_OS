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

void paging_map(uint32_t virt, uint32_t phys, uint32_t flags) {
	uint32_t dir_idx   = virt >> 22;
	uint32_t table_idx = (virt >> 12) & 0x3FF;

	if (!(page_directory[dir_idx] & PAGE_PRESENT)) {
		uint32_t new_table = pmm_alloc_frame();
		memset((void*) new_table, 0, 4096);
		page_directory[dir_idx] = new_table | PAGE_PRESENT | PAGE_WRITE;
	}

	/* Propagate the user bit up to the directory entry */
	page_directory[dir_idx] |= (flags & PAGE_USER);

	uint32_t* table = (uint32_t*) (page_directory[dir_idx] & ~0xFFF);
	table[table_idx] = (phys & ~0xFFF) | flags | PAGE_PRESENT;

	__asm__ volatile ("invlpg (%0)" :: "r"(virt) : "memory");
}

uint32_t paging_virt_to_phys(uint32_t virt) {
	uint32_t dir_idx   = virt >> 22;
	uint32_t table_idx = (virt >> 12) & 0x3FF;

	if (!(page_directory[dir_idx] & PAGE_PRESENT))
		return 0xFFFFFFFF;                   /* no page table */

	uint32_t* table = (uint32_t*) (page_directory[dir_idx] & ~0xFFF);
	if (!(table[table_idx] & PAGE_PRESENT))
		return 0xFFFFFFFF;                   /* not mapped */

	return (table[table_idx] & ~0xFFF) | (virt & 0xFFF);
}
