#include <stdio.h>
#include <stdlib.h>

#include <kernel/tty.h>
#include <kernel/serial.h>
#include <kernel/multiboot.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/tss.h>
#include <kernel/pmm.h>
#include <kernel/paging.h>
#include <kernel/kheap.h>
#include <kernel/pic.h>
#include <kernel/timer.h>
#include <kernel/keyboard.h>
#include <kernel/task.h>
#include <kernel/process.h>
#include <kernel/syscall.h>
#include <kernel/vfs.h>
#include <kernel/initrd.h>
#include <kernel/elf.h>
#include <kernel/debug.h>

extern void jump_usermode(uint32_t entry, uint32_t stack);

#define KERNEL_HEAP_BASE  0xC0000000
#define KERNEL_HEAP_SIZE  0x100000
#define KSTACK_SIZE       4096

#define USER_STACK_BASE   0xB0000000
#define USER_STACK_PAGES  4
#define USER_STACK_TOP    (USER_STACK_BASE + USER_STACK_PAGES * 0x1000)

#define INIT_PROGRAM      "hello.elf"

/* ------------------------------------------------------------------ */

static void init_initrd(struct multiboot_info* mbi) {
	if (!(mbi->flags & MULTIBOOT_INFO_MODS) || mbi->mods_count == 0) {
		printf("no initrd module found\n");
		return;
	}

	struct multiboot_module* mod = (struct multiboot_module*) mbi->mods_addr;

	printf("initrd at 0x%x, %d bytes\n",
	       mod[0].mod_start, mod[0].mod_end - mod[0].mod_start);

	fs_root = initrd_initialize(mod[0].mod_start);
}

static void map_user_stack(void) {
	for (int i = 0; i < USER_STACK_PAGES; i++) {
		uint32_t frame = pmm_alloc_frame();
		if (!frame)
			panic("out of memory mapping user stack");

		paging_map(USER_STACK_BASE + i * 0x1000, frame,
		           PAGE_USER | PAGE_WRITE);
	}
}

static uint32_t load_program(const char* path) {
	struct fs_node* prog = vfs_finddir(fs_root, path);
	if (!prog)
		panic("init program not found in initrd");

	uint8_t* buf = kmalloc(prog->length);
	vfs_read(prog, 0, prog->length, buf);

	uint32_t brk;
	uint32_t entry = elf_load(buf, prog->length, &brk);
	if (!entry)
		panic("failed to load init program");

	/* Must happen AFTER process_initialize() — that memsets the table */
	process_set_brk(brk);

	printf("%s: entry=0x%x brk=0x%x\n", path, entry, brk);
	return entry;
}

/* ------------------------------------------------------------------ */

void kernel_main(uint32_t magic, struct multiboot_info* mbi) {
	/* --- 1. Output, as early as possible --- */
	serial_initialize();
	terminal_initialize();

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
		panic("not booted by a Multiboot loader");

	/* --- 2. Descriptor tables --- */
	gdt_initialize();
	idt_initialize();

	/* --- 3. Memory --- */
	pmm_initialize(mbi);
	paging_initialize();
	kheap_initialize(KERNEL_HEAP_BASE, KERNEL_HEAP_SIZE);

	/* --- 4. Filesystem (needs the heap) --- */
	init_initrd(mbi);

	/* --- 5. Devices and scheduling --- */
	pic_remap(32, 40);
	timer_initialize(100);
	keyboard_initialize();
	tasking_initialize();

	/* --- 6. Process 0: kernel stack for ring-3 interrupts --- */
	uint32_t kstack = (uint32_t) kmalloc_aligned(KSTACK_SIZE) + KSTACK_SIZE;

	tss_set_stack(kstack);
	process_initialize(kstack);

	printf("pid=%d dir=0x%x kstack=0x%x\n",
	       process_current()->pid,
	       process_current()->page_dir,
	       process_current()->kernel_stack);

	/* --- 7. Interrupts on, now that the TSS is valid --- */
	__asm__ volatile ("sti");

	/* --- 8. Load and run the init program --- */
	uint32_t entry = load_program(INIT_PROGRAM);
	map_user_stack();

	jump_usermode(entry, USER_STACK_TOP);

	panic("returned from user mode");
}
