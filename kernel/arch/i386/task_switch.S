.section .text
.global switch_task
.type switch_task, @function

switch_task:
	/* 4(%esp) = old, 8(%esp) = new */
	movl 4(%esp), %eax

	/* --- Save callee-saved registers into old->regs --- */
	movl %ebx,  4(%eax)
	movl %esi, 16(%eax)
	movl %edi, 20(%eax)
	movl %ebp, 28(%eax)

	/* EIP = our return address */
	movl (%esp), %ebx
	movl %ebx, 32(%eax)

	/* ESP = what it will be after ret pops */
	leal 4(%esp), %ebx
	movl %ebx, 24(%eax)

	/* EFLAGS — force IF on so resumed tasks can be preempted */
	pushfl
	popl %ebx
	orl $0x200, %ebx
	movl %ebx, 36(%eax)

	/* CR3 — save the current address space */
        movl %cr3, %ebx
        movl %ebx, 40(%eax)

	/* --- Load the new task --- */
	movl 8(%esp), %eax        /* read BEFORE esp changes */

	movl 24(%eax), %esp       /* switch stacks */

	/* Switch address space only if it actually differs */
	movl 40(%eax), %ecx       /* new cr3 */
	testl %ecx, %ecx
	jz 1f                     /* 0 means "don't touch cr3" */
	movl %cr3, %edx
	cmpl %ecx, %edx
	je 1f
	movl %ecx, %cr3
1:
	movl 32(%eax), %ebx
	pushl %ebx                /* new EIP as a return address */

	movl 36(%eax), %ebx
	pushl %ebx
	popfl                     /* restore EFLAGS */

	movl  4(%eax), %ebx
	movl 16(%eax), %esi
	movl 20(%eax), %edi
	movl 28(%eax), %ebp

	ret                       /* jumps to the new task's EIP */

.size switch_task, . - switch_task
