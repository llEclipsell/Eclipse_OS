#ifndef _KERNEL_SYSCALL_H
#define _KERNEL_SYSCALL_H

#include <stdint.h>
#include <kernel/idt.h>

void syscall_initialize(void);
void syscall_handler(struct registers* regs);
void syscall_set_break(uint32_t brk);

#endif
