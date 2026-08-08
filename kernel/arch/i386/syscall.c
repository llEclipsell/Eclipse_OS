#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/syscall.h>
#include <kernel/idt.h>
#include <kernel/paging.h>
#include <kernel/vfs.h>
#include <kernel/pmm.h>
#include <kernel/process.h>

#define USER_LIMIT     0xC0000000    /* heap and above is kernel territory */
#define USER_BRK_LIMIT 0xB0000000    /* user stack lives here — don't collide */

/* ---- user pointer validation ---------------------------------------- */

static bool user_range_ok(uint32_t addr, uint32_t len) {
	if (len == 0)
		return true;
	if (addr >= USER_LIMIT || addr + len < addr || addr + len > USER_LIMIT)
		return false;

	for (uint32_t a = addr & ~0xFFFU; a < addr + len; a += 0x1000)
		if (!paging_is_user(a))
			return false;

	return true;
}

static bool user_string_ok(const char* s, uint32_t max) {
	for (uint32_t i = 0; i < max; i++) {
		if (!user_range_ok((uint32_t)(s + i), 1))
			return false;
		if (s[i] == '\0')
			return true;
	}
	return false;
}

static struct open_file* get_fd(int fd) {
	struct process* p = process_current();
	if (fd < 0 || fd >= MAX_FDS)
		return NULL;
	return p->fds[fd];
}

/* ---- syscalls -------------------------------------------------------
   Every handler takes the same five machine words so the dispatch table
   needs no function-pointer casts. Unused arguments are discarded. */

static int sys_exit(uint32_t code, uint32_t b, uint32_t c,
                    uint32_t d, uint32_t e) {
	(void) b; (void) c; (void) d; (void) e;

	process_exit((int) code);
	return 0;                                  /* unreachable */
}

static int sys_write(uint32_t fd, uint32_t buf, uint32_t len,
                     uint32_t d, uint32_t e) {
	(void) d; (void) e;

	if (fd != 1 && fd != 2)
		return -EBADF;
	if (!user_range_ok(buf, len))
		return -EFAULT;

	const char* p = (const char*) buf;
	for (uint32_t i = 0; i < len; i++)
		printf("%c", p[i]);

	return (int) len;
}

static int sys_read(uint32_t fd, uint32_t buf, uint32_t len,
                    uint32_t d, uint32_t e) {
	(void) d; (void) e;

	struct open_file* f = get_fd((int) fd);
	if (!f)
		return -EBADF;
	if (!user_range_ok(buf, len))
		return -EFAULT;

	uint32_t got = vfs_read(f->node, f->offset, len, (uint8_t*) buf);
	f->offset += got;

	return (int) got;
}

static int sys_open(uint32_t path, uint32_t flags, uint32_t c,
                    uint32_t d, uint32_t e) {
	(void) flags; (void) c; (void) d; (void) e;

	if (!user_string_ok((const char*) path, 256))
		return -EFAULT;

	struct fs_node* node = vfs_finddir(fs_root, (const char*) path);
	if (!node)
		return -ENOENT;

	struct process* p = process_current();

	int fd = -1;
	for (int i = 3; i < MAX_FDS; i++)
		if (!p->fds[i]) { fd = i; break; }

	if (fd < 0)
		return -EMFILE;                    /* this process is full */

	struct open_file* of = open_file_alloc(node);
	if (!of)
		return -ENFILE;                    /* system-wide table full */

	p->fds[fd] = of;
	return fd;
}

static int sys_close(uint32_t fd, uint32_t b, uint32_t c,
                     uint32_t d, uint32_t e) {
	(void) b; (void) c; (void) d; (void) e;

	struct open_file* f = get_fd((int) fd);
	if (!f)
		return -EBADF;

	process_current()->fds[fd] = NULL;
	open_file_release(f);

	return 0;
}

static int sys_getpid(uint32_t a, uint32_t b, uint32_t c,
                      uint32_t d, uint32_t e) {
	(void) a; (void) b; (void) c; (void) d; (void) e;

	return process_current()->pid;
}

static int sys_sbrk(uint32_t increment, uint32_t b, uint32_t c,
                    uint32_t d, uint32_t e) {
	(void) b; (void) c; (void) d; (void) e;

	int inc = (int) increment;                 /* the register is signed */

	struct process* p = process_current();
	uint32_t old = p->brk;

	if (inc == 0)
		return (int) old;

	if (inc > 0) {
		uint32_t want = old + (uint32_t) inc;

		if (want < old || want > USER_BRK_LIMIT)
			return -ENOMEM;                /* overflow, or into the stack */

		for (uint32_t a = old & ~0xFFFU; a < want; a += 0x1000) {
			if (paging_virt_to_phys(a) != 0xFFFFFFFF)
				continue;                  /* already mapped */

			uint32_t frame = pmm_alloc_frame();
			if (!frame)
				return -ENOMEM;            /* partial growth stays mapped */

			paging_map(a, frame, PAGE_USER | PAGE_WRITE);
			memset((void*) a, 0, 0x1000);
		}

		p->brk = want;
		return (int) old;
	}

	/* Shrinking */
	uint32_t want = old + (uint32_t) inc;      /* inc is negative */

	if (want > old)
		return -EINVAL;                        /* underflowed */

	/* Free only pages entirely above the new break */
	uint32_t first_dead = (want + 0xFFF) & ~0xFFFU;

	for (uint32_t a = first_dead; a < old; a += 0x1000) {
		uint32_t phys = paging_virt_to_phys(a);
		if (phys == 0xFFFFFFFF)
			continue;

		paging_unmap(a);
		pmm_free_frame(phys & ~0xFFFU);
	}

	p->brk = want;
	return (int) old;
}

static int sys_execve(uint32_t path, uint32_t b, uint32_t c,
                      uint32_t d, uint32_t e) {
	(void) b; (void) c; (void) d; (void) e;

	if (!user_string_ok((const char*) path, 256))
		return -EFAULT;

	return process_execve((const char*) path);
}

static int sys_wait(uint32_t status, uint32_t b, uint32_t c,
                    uint32_t d, uint32_t e) {
	(void) b; (void) c; (void) d; (void) e;

	if (status && !user_range_ok(status, sizeof(int)))
		return -EFAULT;

	return process_wait((int*) status);
}

/* ---- dispatch -------------------------------------------------------- */

typedef int (*syscall_fn)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

static syscall_fn syscall_table[SYSCALL_COUNT] = {
	[SYS_EXIT]   = sys_exit,
	[SYS_WRITE]  = sys_write,
	[SYS_READ]   = sys_read,
	[SYS_OPEN]   = sys_open,
	[SYS_CLOSE]  = sys_close,
	[SYS_GETPID] = sys_getpid,
	[SYS_SBRK]   = sys_sbrk,
	[SYS_EXECVE] = sys_execve,
	[SYS_WAIT]   = sys_wait,
};

void syscall_handler(struct registers* regs) {
	/* fork is the only syscall that needs the full saved register state —
	   the child's resume frame is built from it. */
	if (regs->eax == SYS_FORK) {
		regs->eax = (uint32_t) process_fork(regs);
		return;
	}

	if (regs->eax >= SYSCALL_COUNT || !syscall_table[regs->eax]) {
		regs->eax = (uint32_t) -ENOSYS;
		return;
	}

	regs->eax = syscall_table[regs->eax](
		regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
}
