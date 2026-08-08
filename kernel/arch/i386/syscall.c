#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/syscall.h>
#include <kernel/idt.h>
#include <kernel/paging.h>
#include <kernel/vfs.h>
#include <kernel/pmm.h>

#define USER_LIMIT 0xC0000000        /* heap and above is kernel territory */

static bool user_range_ok(uint32_t addr, uint32_t len) {
	if (len == 0)
		return true;
	if (addr >= USER_LIMIT || addr + len < addr || addr + len > USER_LIMIT)
		return false;

	for (uint32_t a = addr & ~0xFFF; a < addr + len; a += 0x1000)
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

#define MAX_FDS 16

struct file_descriptor {
	struct fs_node* node;
	uint32_t        offset;
	bool            used;
};

static struct file_descriptor fd_table[MAX_FDS];

void syscall_initialize(void) {
	memset(fd_table, 0, sizeof(fd_table));

	/* 0, 1, 2 reserved for stdin/stdout/stderr — no node, handled specially */
	fd_table[0].used = fd_table[1].used = fd_table[2].used = true;
}

static int sys_exit(int code) {
	printf("\n[process exited with %d]\n", code);
	for (;;)
		__asm__ volatile ("hlt");
	return 0;                            /* unreachable */
}

static int sys_write(int fd, const char* buf, uint32_t len) {
	if (fd != 1 && fd != 2)
		return -EBADF;
	if (!user_range_ok((uint32_t) buf, len))
		return -EFAULT;

	for (uint32_t i = 0; i < len; i++)
		printf("%c", buf[i]);

	return (int) len;
}

static int sys_open(const char* path, int flags) {
	(void) flags;

	if (!user_string_ok(path, 256))
		return -EFAULT;

	struct fs_node* node = vfs_finddir(fs_root, path);
	if (!node)
		return -ENOENT;

	for (int i = 3; i < MAX_FDS; i++) {
		if (!fd_table[i].used) {
			fd_table[i].node   = node;
			fd_table[i].offset = 0;
			fd_table[i].used   = true;
			return i;
		}
	}
	return -EMFILE;
}

static int sys_read(int fd, char* buf, uint32_t len) {
	if (fd < 3 || fd >= MAX_FDS || !fd_table[fd].used)
		return -EBADF;
	if (!user_range_ok((uint32_t) buf, len))
		return -EFAULT;

	struct file_descriptor* f = &fd_table[fd];
	uint32_t got = vfs_read(f->node, f->offset, len, (uint8_t*) buf);
	f->offset += got;

	return (int) got;
}

static int sys_close(int fd) {
	if (fd < 3 || fd >= MAX_FDS || !fd_table[fd].used)
		return -EBADF;

	fd_table[fd].used = false;
	fd_table[fd].node = 0;
	return 0;
}

static int sys_getpid(void) {
	return 1;                            /* only one process for now */
}

static uint32_t program_break = 0;
static uint32_t break_mapped  = 0;

void syscall_set_break(uint32_t brk) {
	program_break = break_mapped = (brk + 0xFFF) & ~0xFFF;
}

static int sys_sbrk(int increment) {
	uint32_t old = program_break;

	if (increment == 0)
		return (int) old;
	if (increment < 0)
		return -ENOSYS;                  /* shrinking not supported yet */

	uint32_t want = program_break + increment;

	while (break_mapped < want) {
		uint32_t frame = pmm_alloc_frame();
		if (!frame)
			return -ENOMEM;

		paging_map(break_mapped, frame, PAGE_USER | PAGE_WRITE);
		memset((void*) break_mapped, 0, 0x1000);
		break_mapped += 0x1000;
	}

	program_break = want;
	return (int) old;
}

typedef int (*syscall_fn)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

static syscall_fn syscall_table[SYSCALL_COUNT] = {
	[SYS_EXIT]   = (syscall_fn) sys_exit,
	[SYS_WRITE]  = (syscall_fn) sys_write,
	[SYS_READ]   = (syscall_fn) sys_read,
	[SYS_OPEN]   = (syscall_fn) sys_open,
	[SYS_CLOSE]  = (syscall_fn) sys_close,
	[SYS_GETPID] = (syscall_fn) sys_getpid,
	[SYS_SBRK]   = (syscall_fn) sys_sbrk,
};

void syscall_handler(struct registers* regs) {
	if (regs->eax >= SYSCALL_COUNT || !syscall_table[regs->eax]) {
		regs->eax = (uint32_t) -ENOSYS;
		return;
	}

	regs->eax = (uint32_t) syscall_table[regs->eax](
		regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
}
