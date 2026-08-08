#include <string.h>

char* strrchr(const char* s, int c) {
	const char* last = NULL;

	for (;; s++) {
		if (*s == (char) c) last = s;
		if (*s == '\0') break;
	}
	return (char*) last;
}
