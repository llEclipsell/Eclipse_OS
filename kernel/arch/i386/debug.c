#include <stdio.h>
#include <kernel/debug.h>
#include <kernel/kdb.h>
#include <kernel/paging.h>

extern uint32_t kernel_start;
extern uint32_t kernel_end;

struct stackframe {
	struct stackframe* ebp;
	uint32_t eip;
};

const char* symbol_lookup(uint32_t addr, uint32_t* offset) {
	const struct ksymbol* best = 0;

	for (const struct ksymbol* s = ksymbols; s->name; s++)
		if (s->addr <= addr && (!best || s->addr > best->addr))
			best = s;

	if (!best)
		return 0;

	*offset = addr - best->addr;
	return best->name;
}

void stack_trace_from(uint32_t ebp, uint32_t max_frames) {
	struct stackframe* frame = (struct stackframe*) ebp;

	for (uint32_t i = 0; i < max_frames && frame; i++) {
		/* Reject anything unaligned or not actually mapped */
		if ((uint32_t) frame & 3 ||
		    paging_virt_to_phys((uint32_t) frame) == 0xFFFFFFFF) {
			printf("  <bad frame pointer 0x%x>\n", (uint32_t) frame);
			break;
		}
		if (frame->eip < (uint32_t) &kernel_start ||
		    frame->eip > (uint32_t) &kernel_end) {
			printf("  <trace ends at assembly boundary>\n");
			break;
		}

		uint32_t off;
		const char* name = symbol_lookup(frame->eip, &off);

		if (name)
			printf("  [%d] 0x%x  %s+0x%x\n", i, frame->eip, name, off);
		else
			printf("  [%d] 0x%x\n", i, frame->eip);

		frame = frame->ebp;
	}
}

void stack_trace(uint32_t max_frames) {
	uint32_t ebp;
	__asm__ volatile ("movl %%ebp, %0" : "=r"(ebp));
	printf("Stack trace:\n");
	stack_trace_from(ebp, max_frames);
}

void hexdump(uint32_t addr, uint32_t bytes) {
	for (uint32_t i = 0; i < bytes; i += 16) {
		printf("%x: ", addr + i);

		for (uint32_t j = 0; j < 16; j++)
			printf("%x ", *(uint8_t*)(addr + i + j));

		printf(" ");
		for (uint32_t j = 0; j < 16; j++) {
			uint8_t c = *(uint8_t*)(addr + i + j);
			printf("%c", (c >= 32 && c < 127) ? c : '.');
		}
		printf("\n");
	}
}

void dump_registers(struct registers* r) {
	printf("eax=%x ebx=%x ecx=%x edx=%x\n", r->eax, r->ebx, r->ecx, r->edx);
	printf("esi=%x edi=%x ebp=%x esp=%x\n", r->esi, r->edi, r->ebp, r->esp_dummy);
	printf("eip=%x cs=%x eflags=%x\n",      r->eip, r->cs, r->eflags);
}

void panic(const char* msg) {
	__asm__ volatile ("cli");
	printf("\n*** KERNEL PANIC ***\n%s\n", msg);
	stack_trace(16);
	kdb_enter(0);                     /* drop into the console */
	for (;;)
		__asm__ volatile ("hlt");
}
