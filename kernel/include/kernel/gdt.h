#ifndef _KERNEL_GDT_H
#define _KERNEL_GDT_H

#include <stdint.h>

void gdt_initialize(void);
void gdt_set_gate(int num, uint32_t base, uint32_t limit,
                             uint8_t access, uint8_t gran);

#endif
