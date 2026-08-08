.section .text
.global _start
.type _start, @function

_start:
	movl %esp, %ebp

	call main

	pushl %eax
	call exit

1:	jmp 1b                 /* should never get here */

.size _start, . - _start
