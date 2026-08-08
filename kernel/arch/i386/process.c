#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <kernel/process.h>
#include <kernel/paging.h>
#include <kernel/pmm.h>
#include <kernel/kheap.h>
#include <kernel/task.h>
#include <kernel/tss.h>
#include <kernel/elf.h>
#include <kernel/vfs.h>
#include <kernel/idt.h>

#define MAX_OPEN_FILES   32
#define KSTACK_SIZE      4096
#define MAX_COPY_PAGES   1024
#define EXEC_BUF_SIZE    (128 * 1024)

#define USER_STACK_TOP   0xB0004000
#define USER_STACK_PAGES 4

extern void jump_usermode(uint32_t entry, uint32_t stack);
extern void fork_trampoline(void);

static struct process   proc_table[MAX_PROCS];
static struct open_file file_table[MAX_OPEN_FILES];
static int              next_pid = 1;
static uint8_t halt_stack[4096] __attribute__((aligned(16)));

/* The boot address space. Kept in its own variable rather than read back from
   proc_table[0].page_dir, because when pid 0 itself calls execve that field is
   overwritten before the "am I about to free the kernel directory?" check. */
static uint32_t kernel_dir;

/* fork/exec are not reentrant (no SMP, and neither yields mid-operation),
   so static scratch is safe — and avoids leaking, since kheap has no free. */
struct copy_entry { uint32_t virt, frame, flags; };
static struct copy_entry copy_list[MAX_COPY_PAGES];
static uint8_t           exec_buf[EXEC_BUF_SIZE];

/* ---- open_file pool ---- */

struct open_file* open_file_alloc(struct fs_node* node) {
        for (int i = 0; i < MAX_OPEN_FILES; i++) {
                if (file_table[i].refcount == 0) {
                        file_table[i].node     = node;
                        file_table[i].offset   = 0;
                        file_table[i].refcount = 1;
                        return &file_table[i];
                }
        }
        return NULL;
}

void open_file_release(struct open_file* f) {
        if (!f)
                return;
        if (--f->refcount <= 0) {
                f->node     = NULL;
                f->offset   = 0;
                f->refcount = 0;
        }
}

/* ---- process table ---- */

void process_initialize(uint32_t kernel_stack_top) {
        memset(proc_table, 0, sizeof(proc_table));
        memset(file_table, 0, sizeof(file_table));

        /* Process 0 is the context we're already running in — the boot address
           space and the boot kernel stack. It does NOT own a task_t in the
           scheduler ring; main_task in task.c serves that role. */
        struct process* p = &proc_table[0];

        kernel_dir = paging_current_directory();

        p->pid          = 0;
        p->ppid         = 0;
        p->state        = PROC_RUN;
        p->page_dir     = kernel_dir;
        p->kernel_stack = kernel_stack_top;
        p->brk          = 0;
        p->brk_mapped   = 0;

        for (int i = 0; i < MAX_FDS; i++)
                p->fds[i] = NULL;
}

uint32_t process_kernel_directory(void) {
        return kernel_dir;
}

struct process* process_of_task(task_t* t) {
        for (int i = 0; i < MAX_PROCS; i++)
                if (proc_table[i].state != PROC_FREE && &proc_table[i].task == t)
                        return &proc_table[i];
        return NULL;
}

struct process* process_current(void) {
        struct process* p = process_of_task(task_current());

        /* Kernel threads and the boot context have no owning process entry;
           attribute them to process 0 so syscalls always have somewhere to look. */
        return p ? p : &proc_table[0];
}

struct process* process_create(void) {
        for (int i = 1; i < MAX_PROCS; i++) {          /* 0 is reserved */
                if (proc_table[i].state == PROC_FREE) {
                        struct process* p = &proc_table[i];

                        memset(p, 0, sizeof(*p));

                        p->kernel_stack = (uint32_t) kmalloc_aligned(KSTACK_SIZE);
                        if (!p->kernel_stack)
                                return NULL;
                        p->kernel_stack += KSTACK_SIZE;         /* stacks grow down */

                        p->pid   = next_pid++;
                        p->state = PROC_RUN;

                        return p;
                }
        }
        return NULL;
}

struct process* process_by_pid(int pid) {
        for (int i = 0; i < MAX_PROCS; i++)
                if (proc_table[i].state != PROC_FREE && proc_table[i].pid == pid)
                        return &proc_table[i];
        return NULL;
}

void process_set_brk(uint32_t brk) {
        struct process* p = process_current();
        p->brk = p->brk_mapped = (brk + 0xFFF) & ~0xFFF;
}

/* ---- Stage 4: fork ------------------------------------------------- */

int process_fork(struct registers* regs) {
        struct process* parent = process_current();

        struct process* child = process_create();
        if (!child)
                return -EAGAIN;

        child->ppid       = parent->pid;
        child->brk        = parent->brk;
        child->brk_mapped = parent->brk_mapped;

        child->page_dir = paging_new_directory();
        if (!child->page_dir) {
                child->state = PROC_FREE;
                return -ENOMEM;
        }

        /* --- Phase 1: still in the parent — copy every user page --- */
        int n = 0;

        for (uint32_t d = 0; d < KERNEL_DIR_START; d++) {
                if (!(PD_VIRT[d] & PAGE_PRESENT))
                        continue;

                for (uint32_t t = 0; t < 1024; t++) {
                        uint32_t pte = PT_VIRT(d)[t];

                        if (!(pte & PAGE_PRESENT) || !(pte & PAGE_USER))
                                continue;
                        if (n >= MAX_COPY_PAGES)
                                goto out_of_slots;

                        uint32_t virt  = (d << 22) | (t << 12);
                        uint32_t frame = pmm_alloc_frame();
                        if (!frame)
                                goto out_of_slots;

                        paging_tmp_map(frame);
                        memcpy((void*) TMP_VIRT, (const void*) virt, 4096);

                        copy_list[n].virt  = virt;
                        copy_list[n].frame = frame;
                        copy_list[n].flags = pte & 0xFFF;
                        n++;
                }
        }

        /* --- Phase 2: one switch, install everything, switch back --- */
        {
                uint32_t saved = paging_current_directory();

                paging_switch_directory(child->page_dir);
                for (int i = 0; i < n; i++)
                        paging_map(copy_list[i].virt, copy_list[i].frame,
                                   copy_list[i].flags);
                paging_switch_directory(saved);
        }

        /* --- Descriptors: same open_file, so the offset is shared --- */
        for (int i = 0; i < MAX_FDS; i++) {
                child->fds[i] = parent->fds[i];
                if (child->fds[i])
                        child->fds[i]->refcount++;
        }

        /* --- Build the child's resume state --- */
        memset(&child->task, 0, sizeof(child->task));   /* MUST come first */

        {
                uint32_t frame_addr =
                        child->kernel_stack - sizeof(struct registers);

                struct registers* cf = (struct registers*) frame_addr;
                *cf = *regs;
                cf->eax = 0;                     /* the child's fork() returns 0 */

                child->task.regs.esp    = frame_addr;
                child->task.regs.eip    = (uint32_t) fork_trampoline;
                child->task.regs.eflags = 0x002;              /* IF OFF — see below */
                child->task.regs.cr3    = child->page_dir;    /* NOT current */
        }

        task_add(&child->task);

        return child->pid;

out_of_slots:
        for (int i = 0; i < n; i++)
                pmm_free_frame(copy_list[i].frame);

        paging_free_directory(child->page_dir);
        child->state = PROC_FREE;
        return -ENOMEM;
}

/* ---- Stage 5: execve ----------------------------------------------- */

int process_execve(const char* path) {
        struct fs_node* prog = vfs_finddir(fs_root, path);
        if (!prog)
                return -ENOENT;
        if (prog->length > EXEC_BUF_SIZE)
                return -E2BIG;

        /* Read the image BEFORE tearing anything down. exec_buf lives in the
           kernel's .bss, which every address space shares. */
        vfs_read(prog, 0, prog->length, exec_buf);

        struct process* p = process_current();

        uint32_t old_dir = p->page_dir;
        uint32_t new_dir = paging_new_directory();
        if (!new_dir)
                return -ENOMEM;

        paging_switch_directory(new_dir);

        uint32_t brk   = 0;
        uint32_t entry = elf_load(exec_buf, prog->length, &brk);

        if (!entry) {
                paging_switch_directory(old_dir);
                paging_free_directory(new_dir);
                return -ENOEXEC;                /* caller survives */
        }

        p->page_dir = new_dir;

        /* The scheduler reads cr3 from the task_t that is actually in the ring.
           For pid 0 that is main_task in task.c, NOT p->task — so update the
           live one, and keep the process's own copy consistent too. */
        task_current()->regs.cr3 = new_dir;
        p->task.regs.cr3         = new_dir;

        /* Only safe now that it isn't the current address space, and never
           for the boot directory (which pid 0 shares). */
        if (old_dir != kernel_dir)
                paging_free_directory(old_dir);

        p->brk = p->brk_mapped = (brk + 0xFFF) & ~0xFFF;

        /* Fresh user stack */
        for (int i = 0; i < USER_STACK_PAGES; i++) {
                uint32_t frame = pmm_alloc_frame();
                paging_map(USER_STACK_TOP - (i + 1) * 0x1000, frame,
                           PAGE_USER | PAGE_WRITE);
        }

        /* Descriptors survive exec (no FD_CLOEXEC yet) — POSIX behaviour. */

        tss_set_stack(p->kernel_stack);

        jump_usermode(entry, USER_STACK_TOP);
        __builtin_unreachable();
}

/* ---- Stage 6: exit and wait ---------------------------------------- */

void process_exit(int code) {
        struct process* p = process_current();

        __asm__ volatile ("cli");

        if (p->pid == 0) {                  /* the boot context can't exit */
                printf("\n[init exited with %d - halting]\n", code);
                /* The dying process's kernel stack is gone — give the TSS a valid one */
		tss_set_stack((uint32_t) halt_stack + sizeof(halt_stack));

		__asm__ volatile ("sti");
		for (;;)
			__asm__ volatile ("hlt");
        }

        for (int i = 0; i < MAX_FDS; i++) {
                open_file_release(p->fds[i]);
                p->fds[i] = NULL;
        }

        /* Orphans are reparented to process 0 so they can still be reaped */
        for (int i = 0; i < MAX_PROCS; i++)
                if (proc_table[i].state != PROC_FREE && proc_table[i].ppid == p->pid)
                        proc_table[i].ppid = 0;

        p->exit_code = code;
        p->state     = PROC_ZOMBIE;

        /* Leave the address space before destroying it */
        {
                uint32_t dir = p->page_dir;

                paging_switch_directory(kernel_dir);
                p->page_dir              = kernel_dir;
                p->task.regs.cr3         = kernel_dir;
                task_current()->regs.cr3 = kernel_dir;

                if (dir != kernel_dir)
                        paging_free_directory(dir);
        }

        task_remove(&p->task);

        __asm__ volatile ("sti");

        yield();                                /* never comes back */

        for (;;)                                /* belt and braces */
                __asm__ volatile ("hlt");
}

int process_wait(int* status) {
        struct process* self = process_current();

        for (;;) {
                int children = 0;

                for (int i = 0; i < MAX_PROCS; i++) {
                        struct process* c = &proc_table[i];

                        if (c->state == PROC_FREE || c->ppid != self->pid || c == self)
                                continue;

                        children++;

                        if (c->state == PROC_ZOMBIE) {
                                int pid = c->pid;

                                if (status)
                                        *status = c->exit_code;

                                /* Safe to reclaim now — the zombie is off the ring
                                   and no longer running on this stack. */
                                c->state        = PROC_FREE;
                                c->kernel_stack = 0;

                                return pid;
                        }
                }

                if (!children)
                        return -ECHILD;

                yield();                        /* wait for one to exit */
        }
}
