#include <stdio.h>
#include <kernel/tty.h>

void kernel_main(void) {
	terminal_initialize();
	printf("char:   %c\n", 'A');
	printf("string: %s\n", "hello");
	printf("dec:    %d %d %d\n", 0, -42, 2147483647);
	printf("INT_MIN:%d\n", -2147483648);
	printf("uns:    %u\n", 4294967295u);
	printf("hex:    %x %X\n", 0xdeadbeef, 0xdeadbeef);
	printf("oct:    %o\n", 0755);
	printf("ptr:    %p\n", (void*) 0x1000);
	printf("pct:    100%%\n");

}
