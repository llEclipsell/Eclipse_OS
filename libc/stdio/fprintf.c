#include <stdarg.h>
#include <stdio.h>

#include "stdio_impl.h"

static int emit_file(char c, void* ctx) {
	return fputc((unsigned char) c, (FILE*) ctx) == EOF ? -1 : 0;
}

int vfprintf(FILE* f, const char* format, va_list args) {
	return __vprintf_gen(emit_file, f, format, args);
}

int fprintf(FILE* f, const char* format, ...) {
	va_list args;
	va_start(args, format);

	int r = vfprintf(f, format, args);

	va_end(args);
	return r;
}
