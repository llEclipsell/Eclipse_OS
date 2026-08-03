#include <stdio.h>
#include <stdlib.h>
#include <kernel/tty.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/multiboot.h>
#include <kernel/pmm.h>
#include <kernel/paging.h>
#include <kernel/kheap.h>
#include <kernel/pic.h>
#include <kernel/timer.h>
#include <kernel/keyboard.h>
#include <kernel/task.h>

static task_t task_a, task_b;

static void thread_a(void) {
	for (;;) {
		printf("A");
		for (volatile int i = 0; i < 3000000; i++);
	}
}

static void thread_b(void) {
	for (;;) {
		printf("B");
		for (volatile int i = 0; i < 3000000; i++);
	}
}

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
	kheap_initialize(0xC0000000, 0x100000);
	pic_remap(32, 40);
	timer_initialize(100);
	keyboard_initialize();
	tasking_initialize();
	task_create(&task_a, thread_a, 0x202);
	task_create(&task_b, thread_b, 0x202);

	__asm__ volatile ("sti");

	for (;;)
		__asm__ volatile ("hlt");

}
