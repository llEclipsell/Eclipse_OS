#include <stdio.h>
#include <stdlib.h>
#include <kernel/tty.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/multiboot.h>
#include <kernel/pmm.h>
#include <kernel/paging.h>
#include <kernel/kheap.h>

void kernel_main(uint32_t magic, struct multiboot_info* mbi) {
	terminal_initialize();

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		printf("Not booted by a Multiboot loader!\n");
		abort();
	}

	gdt_initialize();
	idt_initialize();
	pmm_initialize(mbi);
	paging_initialize();

	printf("Free frames before kheap: %d\n", pmm_free_frame_count());
	kheap_initialize(0xC0000000, 0x100000);
	printf("Free frames after kheap: %d\n", pmm_free_frame_count());

	uint32_t a = pmm_alloc_frame();
	uint32_t b = pmm_alloc_frame();
	printf("alloc: 0x%x 0x%x\n", a, b);

	pmm_free_frame(a);
	uint32_t c = pmm_alloc_frame();
	printf("realloc: 0x%x (should be 0x%x)\n", c, a);


	char* d = kmalloc(100);
	char* e = kmalloc(50);
	printf("kmalloc: %p %p (used=%d)\n", d, e, kheap_used());

	d[0] = 'X';
	printf("write/read: %c\n", d[0]);

}
