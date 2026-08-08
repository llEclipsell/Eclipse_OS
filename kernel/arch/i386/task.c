#include <stddef.h>
#include <kernel/task.h>
#include <kernel/kheap.h>
#include <string.h>
#include <kernel/process.h>
#include <kernel/tss.h>
#include <kernel/paging.h>
#include <stdio.h>

#define STACK_SIZE 4096

static task_t  main_task;
static task_t* running_task;

void tasking_initialize(void) {
	main_task.next = &main_task;   /* circular list of one */
	running_task   = &main_task;
	main_task.regs.cr3 = paging_current_directory();
}

void task_create(task_t* task, void (*main)(void), uint32_t flags) {
	uint32_t stack = (uint32_t) kmalloc_aligned(STACK_SIZE);
	memset((void*) stack, 0, STACK_SIZE);

	task->regs.eip    = (uint32_t) main;
	task->regs.esp    = stack + STACK_SIZE;   /* stack grows DOWN */
	task->regs.eflags = flags;
	task->regs.cr3    = paging_current_directory();   /* kernel threads share it */
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

	struct process* np = process_of_task(running_task);
	if (np)
		tss_set_stack(np->kernel_stack);

	switch_task(&prev->regs, &running_task->regs);
}

task_t* task_current(void) {
	return running_task;
}

void task_add(task_t* task) {
	task->next         = running_task->next;
	running_task->next = task;
}

void task_remove(task_t* task) {
	task_t* prev = running_task;
	while (prev->next != task) {
		prev = prev->next;
		if (prev == running_task) return;      /* not in the ring */
	}
	prev->next = task->next;
	/* task->next is left intact so a running task can still yield onward */
}

