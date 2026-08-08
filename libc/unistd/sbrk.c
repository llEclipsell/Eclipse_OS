#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>

void* sbrk(intptr_t increment) {
	int ret = syscall(SYS_SBRK, (int) increment, 0, 0, 0, 0);

	if (ret < 0) {
		errno = -ret;
		return (void*) -1;
	}
	return (void*) ret;
}
