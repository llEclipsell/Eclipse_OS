#ifndef _KERNEL_PAGING_H
#define _KERNEL_PAGING_H

#include <stdint.h>
#include <stdbool.h>

#define PAGE_PRESENT 0x1
#define PAGE_WRITE   0x2
#define PAGE_USER    0x4

#define PD_VIRT     ((uint32_t*) 0xFFFFF000)
#define PT_VIRT(i)  ((uint32_t*) (0xFFC00000 + ((i) << 12)))

#define TMP_VIRT    0xFFBFF000     /* scratch slot 1 */
#define TMP2_VIRT   0xFFBFE000     /* scratch slot 2 */
#define KERNEL_DIR_START 768       /* 0xC0000000 >> 22 — first shared kernel entry */

void paging_initialize(void);
void paging_map(uint32_t virt, uint32_t phys, uint32_t flags);
uint32_t paging_virt_to_phys(uint32_t virt);
bool paging_is_user(uint32_t virt);

uint32_t paging_current_directory(void);
uint32_t paging_new_directory(void);
void     paging_switch_directory(uint32_t phys);
void     paging_free_directory(uint32_t phys);
void*    paging_tmp_map(uint32_t phys);
void*    paging_tmp2_map(uint32_t phys);
void     paging_unmap(uint32_t virt);

#endif
