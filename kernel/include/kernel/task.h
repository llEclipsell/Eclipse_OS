#ifndef _KERNEL_TASK_H
#define _KERNEL_TASK_H

#include <stdint.h>

typedef struct {
	uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags, cr3;
} registers_t;

typedef struct task {
	registers_t  regs;
	struct task* next;
} task_t;

void tasking_initialize(void);
void task_create(task_t* task, void (*main)(void), uint32_t flags);
void yield(void);

/* Pure assembly — defined in task_switch.S */
extern void switch_task(registers_t* old, registers_t* new);

#endif
