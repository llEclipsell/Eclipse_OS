#include <stddef.h>
#include <kernel/task.h>
#include <kernel/kheap.h>

#define STACK_SIZE 4096

static task_t  main_task;
static task_t* running_task;

void tasking_initialize(void) {
	main_task.next = &main_task;   /* circular list of one */
	running_task   = &main_task;
}

void task_create(task_t* task, void (*main)(void), uint32_t flags) {
	uint32_t stack = (uint32_t) kmalloc_aligned(STACK_SIZE);

	task->regs.eip    = (uint32_t) main;
	task->regs.esp    = stack + STACK_SIZE;   /* stack grows DOWN */
	task->regs.eflags = flags;
	task->regs.ebx = task->regs.esi = task->regs.edi = task->regs.ebp = 0;

	/* Splice into the circular list after the current task */
	task->next         = running_task->next;
	running_task->next = task;
}

void yield(void) {
	task_t* prev = running_task;
	running_task = running_task->next;

	if (prev == running_task)
		return;

	switch_task(&prev->regs, &running_task->regs);
}
