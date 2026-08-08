#ifndef _STDIO_H
#define _STDIO_H 1

#include <sys/cdefs.h>
#include <stdarg.h>
#include <stddef.h>

#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif
typedef struct _FILE FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

#define EOF (-1)

int fprintf(FILE* stream, const char* format, ...);
int fflush(FILE* stream);
int fputc(int c, FILE* stream);
int fputs(const char* s, FILE* stream);
int printf(const char* __restrict, ...);
int putchar(int);
int puts(const char*);
int vprintf(const char* format, va_list ap);
int vfprintf(FILE* stream, const char* format, va_list ap);
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);

#ifdef __cplusplus
}
#endif

#endif
