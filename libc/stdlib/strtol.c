#include <errno.h>
#include <stdlib.h>

long strtol(const char* nptr, char** endptr, int base) {
	const char* s = nptr;
	int sign = 1;
	long v = 0;

	while (*s == ' ' || *s == '\t' || *s == '\n' ||
	       *s == '\v' || *s == '\f' || *s == '\r') s++;

	if (*s == '-') { sign = -1; s++; }
	else if (*s == '+') s++;

	/* Auto-detect, and accept an explicit 0x for base 16 */
	if ((base == 0 || base == 16) &&
	    s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
		s += 2;
		base = 16;
	} else if (base == 0) {
		base = (*s == '0') ? 8 : 10;
	}

	const char* start = s;

	for (;; s++) {
		int d;

		if (*s >= '0' && *s <= '9')      d = *s - '0';
		else if (*s >= 'a' && *s <= 'z') d = *s - 'a' + 10;
		else if (*s >= 'A' && *s <= 'Z') d = *s - 'A' + 10;
		else break;

		if (d >= base) break;

		v = v * base + d;
	}

	if (endptr)
		*endptr = (char*) (s == start ? nptr : s);

	return sign * v;
}
