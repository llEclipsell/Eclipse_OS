#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct block {
	size_t        size;      /* payload bytes, excluding this header */
	struct block* next;
	int           free;
};

#define HDR sizeof(struct block)

static struct block* head = NULL;

static struct block* find_free(size_t size) {
	for (struct block* b = head; b; b = b->next)
		if (b->free && b->size >= size)
			return b;
	return NULL;
}

static struct block* grow(size_t size) {
	size_t need = size + HDR;
	need = (need + 0xFFF) & ~(size_t) 0xFFF;      /* whole pages */

	void* p = sbrk(need);
	if (p == (void*) -1)
		return NULL;

	struct block* b = (struct block*) p;
	b->size = need - HDR;
	b->next = NULL;
	b->free = 0;

	if (!head) {
		head = b;
	} else {
		struct block* last = head;
		while (last->next) last = last->next;
		last->next = b;
	}

	return b;
}

static void split(struct block* b, size_t size) {
	if (b->size < size + HDR + 8)
		return;                                    /* remainder too small */

	struct block* rest = (struct block*) ((char*) b + HDR + size);
	rest->size = b->size - size - HDR;
	rest->next = b->next;
	rest->free = 1;

	b->size = size;
	b->next = rest;
}

void* malloc(size_t size) {
	if (!size)
		return NULL;

	size = (size + 7) & ~(size_t) 7;               /* 8-byte align */

	struct block* b = find_free(size);

	if (b) {
		split(b, size);
		b->free = 0;
	} else {
		b = grow(size);
		if (!b) return NULL;
		split(b, size);
	}

	return (char*) b + HDR;
}

void free(void* ptr) {
	if (!ptr)
		return;

	struct block* b = (struct block*) ((char*) ptr - HDR);
	b->free = 1;

	/* Coalesce forward with adjacent free blocks */
	while (b->next && b->next->free &&
	       (char*) b + HDR + b->size == (char*) b->next) {
		b->size += HDR + b->next->size;
		b->next  = b->next->next;
	}
}

void* calloc(size_t n, size_t size) {
	size_t total = n * size;
	if (n && total / n != size)
		return NULL;                               /* overflow */

	void* p = malloc(total);
	if (p) memset(p, 0, total);
	return p;
}

void* realloc(void* ptr, size_t size) {
	if (!ptr) return malloc(size);
	if (!size) { free(ptr); return NULL; }

	struct block* b = (struct block*) ((char*) ptr - HDR);
	if (b->size >= size)
		return ptr;

	void* np = malloc(size);
	if (np) {
		memcpy(np, ptr, b->size);
		free(ptr);
	}
	return np;
}
