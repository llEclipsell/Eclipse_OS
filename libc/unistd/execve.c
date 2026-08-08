#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>

int execve(const char* path, char* const argv[], char* const envp[]) {
	(void) argv; (void) envp;          /* not passed through yet */

	int ret = syscall(SYS_EXECVE, (int) path, 0, 0, 0, 0);

	errno = -ret;                      /* only returns on failure */
	return -1;
}
