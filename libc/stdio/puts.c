#include <stdio.h>
#include <string.h>
#include <stddef.h>

#if defined(__is_libk)
#include <kernel/tty.h>
#else
#include <unistd.h>
#endif

int puts(const char* string) {
	size_t len = strlen(string);

#if defined(__is_libk)
	terminal_write(string, len);
	terminal_write("\n", 1);
#else
	if (write(1, string, len) != (ssize_t) len) return EOF;
	if (write(1, "\n", 1) != 1) return EOF;
#endif

	return 1;
}
