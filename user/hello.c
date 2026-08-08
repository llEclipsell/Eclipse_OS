#include <sys/syscall.h>

int syscall(int n, int a1, int a2, int a3, int a4, int a5);

static void puthex(int v) {
	char buf[11] = "0x00000000\n";
	for (int i = 0; i < 8; i++) {
		int nib = (v >> ((7 - i) * 4)) & 0xF;
		buf[2 + i] = nib < 10 ? '0' + nib : 'a' + nib - 10;
	}
	syscall(SYS_WRITE, 1, (int) buf, 11, 0, 0);
}

void _start(void) {
	const char* msg = "syscall test\n";
	syscall(SYS_WRITE, 1, (int) msg, 13, 0, 0);

	/* open / read / close */
	int fd = syscall(SYS_OPEN, (int) "version.txt", 0, 0, 0, 0);
	if (fd >= 0) {
		char buf[64];
		int n = syscall(SYS_READ, fd, (int) buf, 63, 0, 0);
		if (n > 0)
			syscall(SYS_WRITE, 1, (int) buf, n, 0, 0);
		syscall(SYS_CLOSE, fd, 0, 0, 0, 0);
	}

	/* sbrk */
	int brk = syscall(SYS_SBRK, 4096, 0, 0, 0, 0);
	*(int*) brk = 42;

	char buf[64];
	int r;

	//r = syscall(SYS_WRITE, 1, 0x100000, 4, 0, 0);
	//r = syscall(SYS_WRITE, 1, (int) msg, -1, 0, 0);
	//r = syscall(SYS_OPEN, (int) "nope.txt", 0, 0, 0, 0);
	//r = syscall(SYS_READ, 99, (int) buf, 10, 0, 0);
	//r = syscall(42, 0, 0, 0, 0, 0);
	r = syscall(SYS_OPEN, 0xC0000000, 0, 0, 0, 0);
	puthex(r);
	syscall(SYS_EXIT, 0, 0, 0, 0, 0);
	__builtin_unreachable();
}
