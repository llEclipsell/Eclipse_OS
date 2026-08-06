#include <stdint.h>
#include <string.h>
#include <kernel/idt.h>

struct idt_entry {
	uint16_t base_low;
	uint16_t selector;
	uint8_t  always0;
	uint8_t  flags;
	uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr   ip;

extern void idt_flush(uint32_t idt_ptr);

/* Declared in isr.S */
extern void isr0(void);  extern void isr1(void);  extern void isr2(void);
extern void isr3(void);  extern void isr4(void);  extern void isr5(void);
extern void isr6(void);  extern void isr7(void);  extern void isr8(void);
extern void isr9(void);  extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void);
extern void isr15(void); extern void isr16(void); extern void isr17(void);
extern void isr18(void); extern void isr19(void); extern void isr20(void);
extern void isr21(void); extern void isr22(void); extern void isr23(void);
extern void isr24(void); extern void isr25(void); extern void isr26(void);
extern void isr27(void); extern void isr28(void); extern void isr29(void);
extern void isr30(void); extern void isr31(void);

extern void irq0(void);  extern void irq1(void);  extern void irq2(void);
extern void irq3(void);  extern void irq4(void);  extern void irq5(void);
extern void irq6(void);  extern void irq7(void);  extern void irq8(void);
extern void irq9(void);  extern void irq10(void); extern void irq11(void);
extern void irq12(void); extern void irq13(void); extern void irq14(void);
extern void irq15(void);

extern void isr128(void);

static void idt_set_gate(uint8_t num, uint32_t base,
                         uint16_t sel, uint8_t flags) {
	idt[num].base_low  =  base        & 0xFFFF;
	idt[num].base_high = (base >> 16) & 0xFFFF;
	idt[num].selector  = sel;
	idt[num].always0   = 0;
	idt[num].flags     = flags;
}

void idt_initialize(void) {
	ip.limit = (sizeof(struct idt_entry) * 256) - 1;
	ip.base  = (uint32_t) &idt;

	memset(&idt, 0, sizeof(struct idt_entry) * 256);

	void (*stubs[32])(void) = {
		isr0,  isr1,  isr2,  isr3,  isr4,  isr5,  isr6,  isr7,
		isr8,  isr9,  isr10, isr11, isr12, isr13, isr14, isr15,
		isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
		isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
	};

	for (uint8_t i = 0; i < 32; i++)
		idt_set_gate(i, (uint32_t) stubs[i], 0x08, 0x8E);

	void (*irqs[16])(void) = {
		irq0, irq1, irq2,  irq3,  irq4,  irq5,  irq6,  irq7,
		irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15
	};

	for (uint8_t i = 0; i < 16; i++)
		idt_set_gate(32 + i, (uint32_t) irqs[i], 0x08, 0x8E);

	idt_set_gate(0x80, (uint32_t) isr128, 0x08, 0xEE);

	idt_flush((uint32_t) &ip);
}
