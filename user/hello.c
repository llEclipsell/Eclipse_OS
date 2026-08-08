#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

static const char  greeting[] = "Hello from eclipseos libc!";
static const char* gp = greeting;      /* keeps the PIE relocation path alive */

int main(void) {
	printf("%s\n", gp);
	printf("numbers: %d %x %s %c\n", 42, 0xBEEF, "ok", '!');

	char* p = malloc(64);
	strcpy(p, "malloc works");
	puts(p);
	free(p);

	int fd = open("version.txt", O_RDONLY);
	if (fd >= 0) {
		char buf[64];
		int n = read(fd, buf, sizeof(buf) - 1);
		if (n > 0) { buf[n] = '\0'; printf("file: %s", buf); }
		close(fd);
	} else {
		fprintf(stderr, "open failed\n");
	}

	printf("no trailing newline here — flush test");
	return 0;
}
