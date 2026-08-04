#include <stddef.h>
#include <string.h>
#include <kernel/initrd.h>
#include <kernel/kheap.h>
#include <string.h>

#define MAX_FILES 64

struct ustar_header {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];       /* octal ASCII */
	char mtime[12];
	char chksum[8];
	char typeflag;
	char linkname[100];
	char magic[6];       /* "ustar" */
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
	char padding[12];
} __attribute__((packed));

static struct fs_node  root_node;
static struct fs_node  file_nodes[MAX_FILES];
static uint8_t*        file_data[MAX_FILES];
static uint32_t        file_count = 0;
static struct dirent   dirent_buf;

/* Tar stores numbers as octal ASCII */
static uint32_t octal_to_int(const char* s, int len) {
	uint32_t v = 0;
	for (int i = 0; i < len && s[i] >= '0' && s[i] <= '7'; i++)
		v = v * 8 + (s[i] - '0');
	return v;
}

static uint32_t initrd_read(struct fs_node* node, uint32_t offset,
                            uint32_t size, uint8_t* buffer) {
	if (offset >= node->length)
		return 0;
	if (offset + size > node->length)
		size = node->length - offset;

	memcpy(buffer, file_data[node->inode] + offset, size);
	return size;
}

static struct dirent* initrd_readdir(struct fs_node* node, uint32_t index) {
	(void) node;
	if (index >= file_count)
		return NULL;

	strcpy(dirent_buf.name, file_nodes[index].name);
	dirent_buf.inode = index;
	return &dirent_buf;
}

static struct fs_node* initrd_finddir(struct fs_node* node, const char* name) {
	(void) node;
	for (uint32_t i = 0; i < file_count; i++)
		if (!strcmp(file_nodes[i].name, name))
			return &file_nodes[i];
	return NULL;
}

struct fs_node* initrd_initialize(uint32_t location) {
	uint8_t* ptr = (uint8_t*) location;

	while (file_count < MAX_FILES) {
		struct ustar_header* h = (struct ustar_header*) ptr;

		if (h->name[0] == '\0')
			break;                          /* end of archive */
		if (memcmp(h->magic, "ustar", 5) != 0)
			break;                          /* not a ustar header */

		uint32_t size = octal_to_int(h->size, 12);

		if (h->typeflag == '0' || h->typeflag == '\0') {
			struct fs_node* n = &file_nodes[file_count];

			strncpy(n->name, h->name, sizeof(n->name) - 1);
			n->name[sizeof(n->name) - 1] = '\0';
			n->flags  = FS_FILE;
			n->length = size;
			n->inode  = file_count;
			n->read   = initrd_read;
			n->readdir = NULL;
			n->finddir = NULL;

			file_data[file_count] = ptr + 512;
			file_count++;
		}

		/* Header is 512 bytes; data is padded up to a 512 multiple */
		ptr += 512 + ((size + 511) / 512) * 512;
	}

	strcpy(root_node.name, "initrd");
	root_node.flags   = FS_DIRECTORY;
	root_node.length  = 0;
	root_node.read    = NULL;
	root_node.readdir = initrd_readdir;
	root_node.finddir = initrd_finddir;

	return &root_node;
}
