#ifndef _TIME_H
#define _TIME_H 1

#include <sys/cdefs.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t time_t;
typedef int32_t clock_t;

struct timespec {
	time_t tv_sec;
	long   tv_nsec;
};

#ifdef __cplusplus
}
#endif

#endif
