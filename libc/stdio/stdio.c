#include <stdio.h>
#include <unistd.h>
#include "stdio_impl.h"

static struct _FILE _stdout = { 1, _F_WRITE | _F_LINEBUF, 0, {0} };
static struct _FILE _stderr = { 2, _F_WRITE, 0, {0} };   /* unbuffered */
static struct _FILE _stdin  = { 0, 0, 0, {0} };

FILE* stdout = &_stdout;
FILE* stderr = &_stderr;
FILE* stdin  = &_stdin;

int fflush(FILE* f) {
	if (!f || !f->pos)
		return 0;

	ssize_t n = write(f->fd, f->buf, f->pos);
	f->pos = 0;

	if (n < 0) { f->flags |= _F_ERR; return EOF; }
	return 0;
}

int fputc(int c, FILE* f) {
	if (!f || !(f->flags & _F_WRITE))
		return EOF;

	f->buf[f->pos++] = (char) c;

	if (f->pos == FILE_BUFSIZ ||
	    !(f->flags & _F_LINEBUF) ||
	    c == '\n')
		if (fflush(f) == EOF)
			return EOF;

	return c;
}

int fputs(const char* s, FILE* f) {
	while (*s)
		if (fputc(*s++, f) == EOF)
			return EOF;
	return 0;
}

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* f) {
	const unsigned char* p = (const unsigned char*) ptr;
	size_t total = size * nmemb;

	if (!size || !nmemb)
		return 0;

	for (size_t i = 0; i < total; i++)
		if (fputc(p[i], f) == EOF)
			return i / size;          /* whole items written */

	return nmemb;
}

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* f) {
	if (!f || !size || !nmemb)
		return 0;

	ssize_t n = read(f->fd, ptr, size * nmemb);
	if (n <= 0)
		return 0;

	return (size_t) n / size;
}
