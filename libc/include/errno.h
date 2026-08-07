#ifndef _ERRNO_H
#define _ERRNO_H 1

#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int errno;

#define ENOMEM  1
#define EINVAL  2
#define ENOSYS  3
#define EBADF   4
#define ENOENT  5
#define EACCES  6
#define EEXIST  7
#define ERANGE  8

#ifdef __cplusplus
}
#endif

#endif
