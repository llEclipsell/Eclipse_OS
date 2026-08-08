#ifndef _KERNEL_TSS_H
#define _KERNEL_TSS_H

#include <stdint.h>

struct tss_entry {
	uint32_t prev_tss;
	uint32_t esp0;           /* kernel stack pointer on ring 3 → 0 */
	uint32_t ss0;            /* kernel stack segment */
	uint32_t esp1, ss1, esp2, ss2;
	uint32_t cr3, eip, eflags;
	uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
	uint32_t es, cs, ss, ds, fs, gs;
	uint32_t ldt;
	uint16_t trap, iomap_base;
} __attribute__((packed));

void tss_install(int num, uint16_t ss0, uint32_t esp0);
void tss_set_stack(uint32_t esp0);
void tss_flush(void);

#endif
