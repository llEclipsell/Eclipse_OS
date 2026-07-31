#include <stdio.h>
#include <kernel/tty.h>
#include <kernel/gdt.h>

void kernel_main(void) {
	terminal_initialize();
	printf("Terminal online.\n");

	gdt_initialize();
	printf("GDT loaded.\n");

}
