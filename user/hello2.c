#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int marker = 0xAAAA;      /* file-scope: proves .data was reloaded */

int main(void) {
	printf("=== hello2 running ===\n");
	printf("pid=%d marker=0x%x\n", getpid(), marker);

	char* p = malloc(32);
	printf("malloc in new space: %p\n", p);
	free(p);

	return 7;                     /* distinctive exit code for wait() */
}
