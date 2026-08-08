#ifndef _SYS_SYSCALL_H
#define _SYS_SYSCALL_H

#define SYS_EXIT   0
#define SYS_WRITE  1
#define SYS_READ   2
#define SYS_OPEN   3
#define SYS_CLOSE  4
#define SYS_GETPID 5
#define SYS_SBRK   6

#define SYSCALL_COUNT 7

#ifndef __ASSEMBLER__
int syscall(int number, int a1, int a2, int a3, int a4, int a5);
#endif

#endif

