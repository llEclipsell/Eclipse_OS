#ifndef _KERNEL_PROCESS_H
#define _KERNEL_PROCESS_H

#include <stdint.h>
#include <kernel/task.h>
#include <kernel/vfs.h>

struct registers;

#define MAX_FDS      16
#define MAX_PROCS    16

struct open_file {
	struct fs_node* node;
	uint32_t        offset;
	int             refcount;      /* how many descriptors point here */
};

struct process {
	int    pid;
	int    ppid;
	int    state;                  /* RUNNING / ZOMBIE / FREE */
	int    exit_code;

	uint32_t page_dir;             /* physical address for CR3 */
	uint32_t kernel_stack;         /* top, for TSS esp0 */
	uint32_t brk;                  /* program break */
	uint32_t brk_mapped;

	struct open_file* fds[MAX_FDS];

	task_t  task;                  /* scheduler entry */
};

#define PROC_FREE   0
#define PROC_RUN    1
#define PROC_ZOMBIE 2

struct process* process_current(void);
struct process* process_create(void);
int  process_fork(struct registers* regs);
int  process_execve(const char* path);
void process_exit(int code);
int  process_wait(int* status);
struct process* process_of_task(task_t* t);

struct registers;                    /* forward decl — fixes the earlier warning */

void              process_initialize(uint32_t kernel_stack_top);
struct process*   process_of_task(task_t* t);
struct process*   process_by_pid(int pid);
void              process_set_brk(uint32_t brk);

struct open_file* open_file_alloc(struct fs_node* node);
void              open_file_release(struct open_file* f);

#endif
