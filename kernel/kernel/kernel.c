#include <stdio.h>
#include <kernel/tty.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>

void kernel_main(void) {
	terminal_initialize();
	printf("Terminal online.\n");

	gdt_initialize();
	printf("GDT loaded.\n");

	idt_initialize();
	printf("IDT loaded.\n");

	volatile int a = 1, b = 0;
	printf("%d\n", a / b);

}
