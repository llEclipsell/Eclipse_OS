#ifndef _ERRNO_H
#define _ERRNO_H

#include <sys/cdefs.h>

#define EPERM   1
#define ENOENT  2
#define EBADF   9
#define ENOMEM  12
#define EACCES  13
#define EFAULT  14
#define EEXIST  17
#define EINVAL  22
#define EMFILE  24
#define ERANGE  34
#define ENOSYS  38
#define ENFILE 23    /* too many open files in system */
#define ECHILD  10     /* No child processes */
#define EAGAIN  11     /* Try again / resource temporarily unavailable */
#define ENOEXEC  8     /* Exec format error */
#define E2BIG 7      /* Argument list too long */

#ifdef __cplusplus
extern "C" {
#endif

extern int errno;

#ifdef __cplusplus
}
#endif

#endif
