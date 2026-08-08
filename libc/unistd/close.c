#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>

int close(int fd) {
	int ret = syscall(SYS_CLOSE, fd, 0, 0, 0, 0);

	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	return 0;
}
