#ifndef _KERNEL_TIMER_H
#define _KERNEL_TIMER_H

#include <stdint.h>

#define TIMER_QUANTUM 10

void timer_initialize(uint32_t frequency);
uint32_t timer_ticks(void);

#endif
