#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>

ssize_t read(int fd, void* buf, size_t count) {
	int ret = syscall(SYS_READ, fd, (int) buf, (int) count, 0, 0);

	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}
