#include <stdio.h>
#include <kernel/idt.h>
#include <kernel/paging.h>

static int sys_write(uint32_t fd, const char* buf, uint32_t len) {
	(void) fd;

	/* Never trust a user pointer */
	if (paging_virt_to_phys((uint32_t) buf) == 0xFFFFFFFF)
		return -1;

	for (uint32_t i = 0; i < len; i++)
		printf("%c", buf[i]);
	return len;
}

void syscall_handler(struct registers* regs) {
	switch (regs->eax) {
		case 1:
			regs->eax = sys_write(regs->ebx,
			                      (const char*) regs->ecx, regs->edx);
			break;
		case 2:
			printf("\n[process exited with %d]\n", regs->ebx);
			for (;;) __asm__ volatile ("hlt");
		default:
			regs->eax = (uint32_t) -1;
	}
}
