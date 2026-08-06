#define SYS_WRITE 1
#define SYS_EXIT  2

static int sys_write(const char* buf, int len) {
	int ret;
	__asm__ volatile ("int $0x80"
	                  : "=a"(ret)
	                  : "a"(SYS_WRITE), "b"(1), "c"(buf), "d"(len));
	return ret;
}

static void sys_exit(int code) {
	__asm__ volatile ("int $0x80" :: "a"(SYS_EXIT), "b"(code));
}

static int ustrlen(const char* s) {
	int n = 0;
	while (s[n]) n++;
	return n;
}

void _start(void) {
	const char* msg = "Hello from a real ELF program!\n";
	sys_write(msg, ustrlen(msg));
	sys_exit(0);
}
