#ifndef _KERNEL_KDB_H
#define _KERNEL_KDB_H

#include <kernel/idt.h>

void kdb_enter(struct registers* regs);

#endif
