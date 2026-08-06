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
#include <kernel/serial.h>
#include <kernel/debug.h>
#include <kernel/vfs.h>
#include <kernel/initrd.h>
#include <kernel/tss.h>
#include <kernel/elf.h>

extern void jump_usermode(uint32_t entry, uint32_t stack);

void kernel_main(uint32_t magic, struct multiboot_info* mbi) {
	serial_initialize();
	serial_write("SERIAL OK\n");

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

	if (mbi->flags & MULTIBOOT_INFO_MODS && mbi->mods_count > 0) {
		struct multiboot_module* mod =
			(struct multiboot_module*) mbi->mods_addr;

		printf("initrd at 0x%x, %d bytes\n",
		       mod[0].mod_start, mod[0].mod_end - mod[0].mod_start);

		fs_root = initrd_initialize(mod[0].mod_start);
	} else {
		printf("no initrd module found\n");
	}

	pic_remap(32, 40);
	timer_initialize(100);
	keyboard_initialize();
	tasking_initialize();
	//task_create(&task_a, thread_a, 0x202);
	//task_create(&task_b, thread_b, 0x202);

	__asm__ volatile ("sti");

	struct fs_node* prog = vfs_finddir(fs_root, "hello.elf");
	if (!prog) {
		printf("hello.elf not found in initrd\n");
		abort();
	}

	uint8_t* buf = kmalloc(prog->length);
	vfs_read(prog, 0, prog->length, buf);

	uint32_t entry = elf_load(buf, prog->length);
	if (!entry) {
		printf("failed to load hello.elf\n");
		abort();
	}

	uint32_t ustack = pmm_alloc_frame();
	paging_map(0xB0000000, ustack, PAGE_USER | PAGE_WRITE);

	/* Kernel stack for interrupts arriving from ring 3 */
	uint32_t kstack = (uint32_t) kmalloc_aligned(4096);
	tss_set_stack(kstack + 4096);

	printf("loaded hello.elf, entry = 0x%x\n", entry);
	jump_usermode(entry, 0xB0001000);

}
