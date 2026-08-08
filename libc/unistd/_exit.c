#include <sys/syscall.h>
#include <unistd.h>

void _exit(int status) {
	syscall(SYS_EXIT, status, 0, 0, 0, 0);

	__builtin_unreachable();
}
