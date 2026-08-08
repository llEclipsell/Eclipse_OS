#include <stdlib.h>
#include <string.h>

char* strdup(const char* s) {
	size_t len = strlen(s) + 1;
	char* p = malloc(len);

	if (p) memcpy(p, s, len);
	return p;
}
