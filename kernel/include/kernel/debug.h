#ifndef _KERNEL_DEBUG_H
#define _KERNEL_DEBUG_H

#define BREAKPOINT() __asm__ volatile ("int3")

#include <stdint.h>
#include <kernel/idt.h>

struct ksymbol {
	uint32_t    addr;
	const char* name;
};

extern const struct ksymbol ksymbols[];

const char* symbol_lookup(uint32_t addr, uint32_t* offset);
void        stack_trace(uint32_t max_frames);
void        hexdump(uint32_t addr, uint32_t bytes);
void        dump_registers(struct registers* r);

void panic(const char* msg) __attribute__((noreturn));

#define ASSERT(cond) \
	((cond) ? (void)0 : panic("Assertion failed: " #cond \
	                          " at " __FILE__))

#endif
