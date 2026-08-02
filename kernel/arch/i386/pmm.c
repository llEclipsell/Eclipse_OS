#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <kernel/pmm.h>
#include <kernel/multiboot.h>

/* Linker-provided markers — see linker.ld */
extern uint32_t kernel_start;
extern uint32_t kernel_end;

static uint32_t* frame_bitmap;
static uint32_t  total_frames;

/* --- bitmap helpers --- */

static void frame_set(uint32_t frame)   { frame_bitmap[frame / 32] |=  (1 << (frame % 32)); }
static void frame_clear(uint32_t frame) { frame_bitmap[frame / 32] &= ~(1 << (frame % 32)); }
static bool frame_test(uint32_t frame)  { return frame_bitmap[frame / 32] & (1 << (frame % 32)); }

/* --- initialization --- */

void pmm_initialize(struct multiboot_info* mbi) {
	uint32_t mmap_end = mbi->mmap_addr + mbi->mmap_length;

	/* Pass 1: find the top of usable RAM */
	uint32_t highest = 0;
	struct multiboot_mmap_entry* e =
		(struct multiboot_mmap_entry*) mbi->mmap_addr;

	while ((uint32_t) e < mmap_end) {
		if (e->type == MULTIBOOT_MEMORY_AVAILABLE) {
			uint32_t top = (uint32_t) (e->addr + e->len);
			if (top > highest) highest = top;
		}
		e = (struct multiboot_mmap_entry*)
		    ((uint32_t) e + e->size + sizeof(e->size));
	}

	total_frames = highest / PAGE_SIZE;
	uint32_t bitmap_bytes = (total_frames + 7) / 8;

	frame_bitmap = (uint32_t*) &kernel_end;

	/* Everything used until proven otherwise */
	memset(frame_bitmap, 0xFF, bitmap_bytes);

	/* Pass 2: free the available regions */
	e = (struct multiboot_mmap_entry*) mbi->mmap_addr;
	while ((uint32_t) e < mmap_end) {
		if (e->type == MULTIBOOT_MEMORY_AVAILABLE) {
			uint32_t base = (uint32_t) e->addr;
			uint32_t len  = (uint32_t) e->len;
			for (uint32_t a = base; a < base + len; a += PAGE_SIZE)
				frame_clear(a / PAGE_SIZE);
		}
		e = (struct multiboot_mmap_entry*)
		    ((uint32_t) e + e->size + sizeof(e->size));
	}

	/* Pass 3: reserve what must never be handed out */
	uint32_t kstart  = (uint32_t) &kernel_start;
	uint32_t bmp_end = (uint32_t) &kernel_end + bitmap_bytes;

	for (uint32_t a = 0; a < 0x100000; a += PAGE_SIZE)
		frame_set(a / PAGE_SIZE);              /* low 1 MiB */

	for (uint32_t a = kstart; a < bmp_end; a += PAGE_SIZE)
		frame_set(a / PAGE_SIZE);              /* kernel + bitmap */
}

/* --- allocation --- */

uint32_t pmm_alloc_frame(void) {
	for (uint32_t i = 0; i < total_frames; i++)
		if (!frame_test(i)) {
			frame_set(i);
			return i * PAGE_SIZE;
		}
	return 0;  /* out of memory */
}

void pmm_free_frame(uint32_t addr) {
	frame_clear(addr / PAGE_SIZE);
}

uint32_t pmm_free_frame_count(void) {
	uint32_t count = 0;
	for (uint32_t i = 0; i < total_frames; i++)
		if (!frame_test(i))
			count++;
	return count;
}
