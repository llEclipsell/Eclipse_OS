#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>

pid_t fork(void) {
	int ret = syscall(SYS_FORK, 0, 0, 0, 0, 0);

	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}
