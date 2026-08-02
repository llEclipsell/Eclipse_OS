#ifndef _KERNEL_PMM_H
#define _KERNEL_PMM_H

#include <stdint.h>
#include <kernel/multiboot.h>

#define PAGE_SIZE 4096

void     pmm_initialize(struct multiboot_info* mbi);
uint32_t pmm_alloc_frame(void);
void     pmm_free_frame(uint32_t addr);
uint32_t pmm_free_frame_count(void);

#endif
