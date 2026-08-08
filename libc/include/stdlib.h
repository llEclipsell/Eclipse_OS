#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <sys/cdefs.h>
#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((__noreturn__))
void abort(void);

/* Hosted only — these need syscalls or the heap */
#if !defined(__is_libk)

__attribute__((__noreturn__))
void exit(int status);

void* malloc(size_t size);
void* calloc(size_t nmemb, size_t size);
void* realloc(void* ptr, size_t size);
void  free(void* ptr);

int   atoi(const char* nptr);
long  atol(const char* nptr);
int   abs(int j);

#endif

#ifdef __cplusplus
}
#endif

#endif
