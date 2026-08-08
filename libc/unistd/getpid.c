#include <sys/syscall.h>
#include <unistd.h>

pid_t getpid(void) {
	return syscall(SYS_GETPID, 0, 0, 0, 0, 0);
}
