#ifndef _KERNEL_IRQ_H
#define _KERNEL_IRQ_H

#include <stdint.h>
#include <kernel/idt.h>

typedef void (*irq_handler_t)(struct registers*);

void irq_install_handler(uint8_t irq, irq_handler_t handler);
void irq_handler(struct registers* regs);

#endif
