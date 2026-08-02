#ifndef _KERNEL_KHEAP_H
#define _KERNEL_KHEAP_H

#include <stddef.h>
#include <stdint.h>

void  kheap_initialize(uint32_t start, uint32_t size);
void* kmalloc(size_t size);
void* kmalloc_aligned(size_t size);
uint32_t kheap_used(void);

#endif
