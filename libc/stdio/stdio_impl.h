#ifndef _STDIO_IMPL_H
#define _STDIO_IMPL_H

#include <stddef.h>
#include <stdarg.h>

#define FILE_BUFSIZ 256

#define _F_WRITE   0x01
#define _F_LINEBUF 0x02
#define _F_ERR     0x04

struct _FILE {
	int    fd;
	int    flags;
	size_t pos;
	char   buf[FILE_BUFSIZ];
};

typedef int (*emit_fn)(char c, void* ctx);	/* 0 = ok, -1 = failed */

int __vprintf_gen(emit_fn emit, void* ctx,
                  const char* format, va_list parameters);

#endif
