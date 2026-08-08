#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

static const char  greeting[] = "Hello from eclipseos libc!";
static const char* gp = greeting;      /* keeps the PIE relocation path alive */

int main(void) {
	printf("%s\n", gp);
	printf("parent: pid=%d\n", getpid());

	/* --- Test 1: fork + execve, child exits with hello2's code --- */
	int pid = fork();
	if (pid < 0) {
		printf("fork failed: %d\n", pid);
		return 1;
	}
	if (pid == 0) {
		execve("hello2.elf", NULL, NULL);
		printf("child: exec failed\n");
		exit(1);
	}

	int status;
	int reaped = wait(&status);
	printf("test1: reaped %d, status %d (want 7)\n", reaped, status);

	/* --- Test 2: fork + exit(42), no exec involved --- */
	pid = fork();
	if (pid == 0)
		exit(42);

	reaped = wait(&status);
	printf("test2: reaped %d, status %d (want 42)\n", reaped, status);

	/* --- Test 3: wait with nothing left to reap --- */
	int r = wait(&status);
	printf("test3: wait returned %d (want negative)\n", r);

	return 0;
}
