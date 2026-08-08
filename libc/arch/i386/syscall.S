.section .text
.global syscall
.type syscall, @function

syscall:
	pushl %ebx
	pushl %esi
	pushl %edi

	movl 16(%esp), %eax        /* number */
	movl 20(%esp), %ebx        /* arg1 */
	movl 24(%esp), %ecx        /* arg2 */
	movl 28(%esp), %edx        /* arg3 */
	movl 32(%esp), %esi        /* arg4 */
	movl 36(%esp), %edi        /* arg5 */

	int $0x80

	popl %edi
	popl %esi
	popl %ebx
	ret

.size syscall, . - syscall
