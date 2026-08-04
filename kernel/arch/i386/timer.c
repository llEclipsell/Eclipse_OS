#include <stdio.h>
#include <kernel/io.h>
#include <kernel/irq.h>
#include <kernel/task.h>
#include <kernel/pic.h>
#include <kernel/timer.h>

static volatile uint32_t tick = 0;

uint32_t timer_ticks(void) {
	return tick;
}

static void timer_callback(struct registers* regs) {
	(void) regs;
	tick++;

	if (tick % TIMER_QUANTUM == 0)
		yield();
}

void timer_initialize(uint32_t frequency) {
	uint32_t divisor = 1193180 / frequency;

	outb(0x43, 0x36);                        /* channel 0, mode 3 */
	outb(0x40, divisor & 0xFF);              /* low byte */
	outb(0x40, (divisor >> 8) & 0xFF);       /* high byte */

	irq_install_handler(0, timer_callback);
}
