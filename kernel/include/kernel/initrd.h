#ifndef _KERNEL_INITRD_H
#define _KERNEL_INITRD_H

#include <kernel/vfs.h>

struct fs_node* initrd_initialize(uint32_t location);

#endif
