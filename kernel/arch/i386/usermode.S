.section .text
.global jump_usermode
.type jump_usermode, @function

jump_usermode:
	movl 4(%esp), %eax        /* entry point */
	movl 8(%esp), %ebx        /* user stack top */

	movw $0x23, %cx           /* user data selector | RPL 3 */
	movw %cx, %ds
	movw %cx, %es
	movw %cx, %fs
	movw %cx, %gs

	pushl $0x23               /* SS */
	pushl %ebx                /* ESP */

	pushfl                    /* EFLAGS */
	popl %ecx
	orl $0x200, %ecx          /* set IF so we stay preemptible */
	pushl %ecx

	pushl $0x1B               /* CS = user code | RPL 3 */
	pushl %eax                /* EIP */

	iret

.size jump_usermode, . - jump_usermode
