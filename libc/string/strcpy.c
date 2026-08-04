#include <string.h>

char* strcpy(char* __restrict dest, const char* __restrict src) {
	char* d = dest;
	while ((*d++ = *src++))
		;
	return dest;
}
