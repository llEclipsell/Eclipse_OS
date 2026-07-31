#include <stdio.h>
#include <stdlib.h>
#include <kernel/idt.h>

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
	printf("\n=== EXCEPTION %d: %s ===\n",
	       regs->int_no, exception_messages[regs->int_no]);
	printf("err=0x%x  eip=0x%x  cs=0x%x  eflags=0x%x\n",
	       regs->err_code, regs->eip, regs->cs, regs->eflags);
	printf("eax=0x%x ebx=0x%x ecx=0x%x edx=0x%x\n",
	       regs->eax, regs->ebx, regs->ecx, regs->edx);
	abort();
}
