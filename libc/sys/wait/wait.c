#include <errno.h>
#include <sys/syscall.h>
#include <sys/wait.h>

pid_t wait(int* status) {
	int ret = syscall(SYS_WAIT, (int) status, 0, 0, 0, 0);

	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}
