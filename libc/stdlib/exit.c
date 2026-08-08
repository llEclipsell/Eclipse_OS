#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void exit(int status) {
	fflush(stdout);
	fflush(stderr);
	_exit(status);
	__builtin_unreachable();
}
