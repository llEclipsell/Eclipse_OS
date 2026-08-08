.section .text
.global fork_trampoline
.type fork_trampoline, @function

/* Entered by switch_task with %esp pointing at a struct registers.
   Identical to the tail of isr_common. */
fork_trampoline:
	popl %eax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	popa
	addl $8, %esp          /* discard int_no and err_code */
	iret

.size fork_trampoline, . - fork_trampoline
