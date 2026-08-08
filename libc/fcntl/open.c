#include <errno.h>
#include <fcntl.h>
#include <sys/syscall.h>

int open(const char* path, int flags, ...) {
	int ret = syscall(SYS_OPEN, (int) path, flags, 0, 0, 0);

	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}
