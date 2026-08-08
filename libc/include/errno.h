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

#ifdef __cplusplus
extern "C" {
#endif

extern int errno;

#ifdef __cplusplus
}
#endif

#endif
