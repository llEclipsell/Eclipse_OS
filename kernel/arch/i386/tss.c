#include <string.h>
#include <kernel/tss.h>
#include <kernel/gdt.h>

static struct tss_entry tss;

void tss_install(int num, uint16_t ss0, uint32_t esp0) {
	uint32_t base  = (uint32_t) &tss;
	uint32_t limit = sizeof(tss) - 1;

	memset(&tss, 0, sizeof(tss));
	tss.ss0  = ss0;
	tss.esp0 = esp0;
	tss.iomap_base = sizeof(tss);   /* no I/O permission bitmap */

	gdt_set_gate(num, base, limit, 0x89, 0x00);
}

void tss_set_stack(uint32_t esp0) {
	tss.esp0 = esp0;
}
