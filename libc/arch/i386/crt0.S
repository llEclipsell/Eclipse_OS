.section .text
.global _start
.type _start, @function

_start:
	movl %esp, %ebp

	call main

	/* main's return value becomes the exit code */
	movl %eax, %ebx
	movl $0, %eax          /* SYS_EXIT */
	int $0x80

1:	jmp 1b                 /* should never get here */

.size _start, . - _start
