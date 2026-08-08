#ifndef _UNISTD_H
#define _UNISTD_H 1

#include <sys/cdefs.h>
#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

ssize_t write(int fd, const void* buf, size_t count);
ssize_t read(int fd, void* buf, size_t count);
int     close(int fd);
pid_t   getpid(void);
void*   sbrk(intptr_t increment);

void _exit(int status) __attribute__((noreturn));

pid_t fork(void);
int   execve(const char* path, char* const argv[], char* const envp[]);

#ifdef __cplusplus
}
#endif

#endif
