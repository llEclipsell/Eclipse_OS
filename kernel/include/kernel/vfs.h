#ifndef _KERNEL_VFS_H
#define _KERNEL_VFS_H

#include <stdint.h>

#define FS_FILE      0x01
#define FS_DIRECTORY 0x02

struct fs_node;

typedef uint32_t (*read_fn)(struct fs_node*, uint32_t offset,
                            uint32_t size, uint8_t* buffer);
typedef struct dirent* (*readdir_fn)(struct fs_node*, uint32_t index);
typedef struct fs_node* (*finddir_fn)(struct fs_node*, const char* name);

struct fs_node {
	char     name[128];
	uint32_t flags;
	uint32_t length;
	uint32_t inode;          /* filesystem-specific index */

	read_fn    read;
	readdir_fn readdir;
	finddir_fn finddir;
};

struct dirent {
	char     name[128];
	uint32_t inode;
};

extern struct fs_node* fs_root;

uint32_t        vfs_read(struct fs_node* node, uint32_t offset,
                         uint32_t size, uint8_t* buffer);
struct dirent*  vfs_readdir(struct fs_node* node, uint32_t index);
struct fs_node* vfs_finddir(struct fs_node* node, const char* name);

#endif
