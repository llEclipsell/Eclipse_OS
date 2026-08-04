#include <stddef.h>
#include <kernel/vfs.h>

struct fs_node* fs_root = NULL;

uint32_t vfs_read(struct fs_node* node, uint32_t offset,
                  uint32_t size, uint8_t* buffer) {
	return node && node->read ? node->read(node, offset, size, buffer) : 0;
}

struct dirent* vfs_readdir(struct fs_node* node, uint32_t index) {
	if (!node || !(node->flags & FS_DIRECTORY) || !node->readdir)
		return NULL;
	return node->readdir(node, index);
}

struct fs_node* vfs_finddir(struct fs_node* node, const char* name) {
	if (!node || !(node->flags & FS_DIRECTORY) || !node->finddir)
		return NULL;
	return node->finddir(node, name);
}
