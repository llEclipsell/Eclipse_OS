#include <stdio.h>
#include <stdlib.h>
#include <kernel/idt.h>
#include <kernel/kdb.h>
#include <kernel/debug.h>

static const char* exception_messages[] = {
	"Divide by zero",
	"Debug",
	"Non-maskable interrupt",
	"Breakpoint",
	"Overflow",
	"Bound range exceeded",
	"Invalid opcode",
	"Device not available",
	"Double fault",
	"Coprocessor segment overrun",
	"Invalid TSS",
	"Segment not present",
	"Stack-segment fault",
	"General protection fault",
	"Page fault",
	"Reserved",
	"x87 floating-point exception",
	"Alignment check",
	"Machine check",
	"SIMD floating-point exception",
	"Virtualization exception",
	"Control protection exception",
	"Reserved", "Reserved", "Reserved", "Reserved",
	"Reserved", "Reserved",
	"Hypervisor injection exception",
	"VMM communication exception",
	"Security exception",
	"Reserved"
};

void isr_handler(struct registers* regs) {
	if (regs->int_no == 128) {
		syscall_handler(regs);
		return;
	}
	if (regs->int_no == 14) {
		uint32_t cr2;
		__asm__ volatile ("movl %%cr2, %0" : "=r"(cr2));
		printf("\n=== PAGE FAULT at 0x%x ===\n", cr2);
		printf("%s, %s, %s\n",
		       regs->err_code & 0x1 ? "protection violation" : "not present",
		       regs->err_code & 0x2 ? "write" : "read",
		       regs->err_code & 0x4 ? "user" : "kernel");
		printf("eip=0x%x\n", regs->eip);
		abort();
	}

	if (regs->int_no == 3) {
		printf("--- breakpoint at 0x%x ---\n", regs->eip);
		kdb_enter(regs);
		return;
	}

	printf("\n=== EXCEPTION %d: %s ===\n",
	       regs->int_no, exception_messages[regs->int_no]);
	printf("err=0x%x  eip=0x%x  cs=0x%x  eflags=0x%x\n",
	       regs->err_code, regs->eip, regs->cs, regs->eflags);
	printf("eax=0x%x ebx=0x%x ecx=0x%x edx=0x%x\n",
	       regs->eax, regs->ebx, regs->ecx, regs->edx);

	stack_trace(16);
	kdb_enter(regs);
	abort();
}
