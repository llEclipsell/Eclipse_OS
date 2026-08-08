#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>

ssize_t write(int fd, const void* buf, size_t count) {
	int ret = syscall(SYS_WRITE, fd, (int) buf, (int) count, 0, 0);

	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}
