#include <string.h>
#include <kernel/paging.h>
#include <kernel/pmm.h>
#include <stdbool.h>

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

	page_directory[0]    = ((uint32_t) first_table)    | PAGE_PRESENT | PAGE_WRITE;
	page_directory[1023] = ((uint32_t) page_directory) | PAGE_PRESENT | PAGE_WRITE;

	paging_load((uint32_t) page_directory);
}

static inline void invlpg(uint32_t virt) {
	__asm__ volatile ("invlpg (%0)" :: "r"(virt) : "memory");
}

void paging_map(uint32_t virt, uint32_t phys, uint32_t flags) {
	uint32_t d = virt >> 22;
	uint32_t t = (virt >> 12) & 0x3FF;

	if (!(PD_VIRT[d] & PAGE_PRESENT)) {
		uint32_t frame = pmm_alloc_frame();
		PD_VIRT[d] = frame | PAGE_PRESENT | PAGE_WRITE;

		invlpg((uint32_t) PT_VIRT(d));
		memset(PT_VIRT(d), 0, 4096);
	}

	PD_VIRT[d] |= (flags & PAGE_USER);
	PT_VIRT(d)[t] = (phys & ~0xFFF) | flags | PAGE_PRESENT;

	invlpg(virt);
}

void paging_unmap(uint32_t virt) {
	uint32_t d = virt >> 22;
	if (!(PD_VIRT[d] & PAGE_PRESENT)) return;

	PT_VIRT(d)[(virt >> 12) & 0x3FF] = 0;
	invlpg(virt);
}

uint32_t paging_virt_to_phys(uint32_t virt) {
	uint32_t d = virt >> 22;
	uint32_t t = (virt >> 12) & 0x3FF;

	if (!(PD_VIRT[d] & PAGE_PRESENT))
		return 0xFFFFFFFF;

	uint32_t pte = PT_VIRT(d)[t];
	if (!(pte & PAGE_PRESENT))
		return 0xFFFFFFFF;

	return (pte & ~0xFFF) | (virt & 0xFFF);
}

bool paging_is_user(uint32_t virt) {
	uint32_t d    = virt >> 22;
	uint32_t t    = (virt >> 12) & 0x3FF;
	uint32_t need = PAGE_PRESENT | PAGE_USER;

	if ((PD_VIRT[d] & need) != need)
		return false;

	return (PT_VIRT(d)[t] & need) == need;
}

void* paging_tmp_map(uint32_t phys) {
	paging_map(TMP_VIRT, phys, PAGE_WRITE);
	return (void*) TMP_VIRT;
}

void* paging_tmp2_map(uint32_t phys) {
	paging_map(TMP2_VIRT, phys, PAGE_WRITE);
	return (void*) TMP2_VIRT;
}

uint32_t paging_new_directory(void) {
	uint32_t frame = pmm_alloc_frame();

	uint32_t* nd = (uint32_t*) paging_tmp_map(frame);
	memset(nd, 0, 4096);

	/* Share every kernel mapping */
	nd[0] = PD_VIRT[0];                                  /* identity 4 MiB */
	for (int i = KERNEL_DIR_START; i < 1023; i++)
		nd[i] = PD_VIRT[i];                              /* heap and up */

	nd[1023] = frame | PAGE_PRESENT | PAGE_WRITE;        /* recursive */

	return frame;
}

uint32_t paging_current_directory(void) {
	uint32_t cr3;
	__asm__ volatile ("movl %%cr3, %0" : "=r"(cr3));
	return cr3 & ~0xFFF;
}

void paging_switch_directory(uint32_t phys) {
	__asm__ volatile ("movl %0, %%cr3" :: "r"(phys) : "memory");
}

void paging_free_directory(uint32_t phys) {
	uint32_t* pd = (uint32_t*) paging_tmp_map(phys);

	/* Entry 0 is the shared kernel identity map — never free it.
	   Entries 768..1022 are shared kernel space. 1023 is recursive. */
	for (uint32_t d = 1; d < KERNEL_DIR_START; d++) {
		if (!(pd[d] & PAGE_PRESENT))
			continue;

		uint32_t table_frame = pd[d] & ~0xFFF;
		uint32_t* pt = (uint32_t*) paging_tmp2_map(table_frame);

		for (uint32_t t = 0; t < 1024; t++)
			if (pt[t] & PAGE_PRESENT)
				pmm_free_frame(pt[t] & ~0xFFF);

		pmm_free_frame(table_frame);
	}

	pmm_free_frame(phys);

	paging_unmap(TMP_VIRT);
	paging_unmap(TMP2_VIRT);
}
