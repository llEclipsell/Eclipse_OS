#include <kernel/idt.h>
#include <kernel/pic.h>
#include <kernel/irq.h>

static irq_handler_t handlers[16] = { 0 };

void irq_install_handler(uint8_t irq, irq_handler_t handler) {
	handlers[irq] = handler;
}

void irq_handler(struct registers* regs) {
	uint8_t irq = regs->int_no - 32;

	if (handlers[irq])
		handlers[irq](regs);

	pic_send_eoi(irq);
}
