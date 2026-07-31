#ifndef _KERNEL_IDT_H
#define _KERNEL_IDT_H

#include <stdint.h>

struct registers {
	uint32_t ds;
	uint32_t edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax;
	uint32_t int_no, err_code;
	uint32_t eip, cs, eflags, useresp, ss;
};

void idt_initialize(void);

#endif
