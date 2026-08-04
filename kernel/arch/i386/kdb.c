#include <stdio.h>
#include <string.h>
#include <kernel/io.h>
#include <kernel/kdb.h>
#include <kernel/debug.h>
#include <kernel/pmm.h>
#include <kernel/kheap.h>
#include <kernel/paging.h>
#include <kernel/kdb.h>
#include <kernel/task.h>
#include <kernel/vfs.h>
#include <string.h>
#include <stdbool.h>
#include <kernel/tty.h>

static const char kdb_keys[128] = {
	0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
	'\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
	0,'a','s','d','f','g','h','j','k','l',';','\'','`',
	0,'\\','z','x','c','v','b','n','m',',','.','/', 0, '*', 0, ' ',
};

#define KDB_KEY_UP    0x100
#define KDB_KEY_DOWN  0x101
#define KDB_KEY_LEFT  0x102
#define KDB_KEY_RIGHT 0x103

#define HIST_SIZE 16

static char hist[HIST_SIZE][80];
static int  hist_count = 0;    /* entries stored, capped at HIST_SIZE */
static int  hist_next  = 0;    /* where the next one goes */

/* pos 0 = most recent */
static const char* hist_get(int pos) {
	int idx = (hist_next - 1 - pos + HIST_SIZE * 2) % HIST_SIZE;
	return hist[idx];
}

static void hist_add(const char* line) {
	if (!line[0])
		return;
	if (hist_count && !strcmp(hist_get(0), line))
		return;                              /* skip consecutive dupes */

	strncpy(hist[hist_next], line, 79);
	hist[hist_next][79] = 0;
	hist_next = (hist_next + 1) % HIST_SIZE;
	if (hist_count < HIST_SIZE)
		hist_count++;
}

static int kdb_getchar(void) {
	bool ext = false;

	for (;;) {
		if (!(inb(0x64) & 1))
			continue;

		uint8_t sc = inb(0x60);

		if (sc == 0xE0) { ext = true;  continue; }
		if (sc & 0x80)  { ext = false; continue; }   /* release */

		if (ext) {
			ext = false;
			switch (sc) {
				case 0x48: return KDB_KEY_UP;
				case 0x50: return KDB_KEY_DOWN;
				case 0x4B: return KDB_KEY_LEFT;
				case 0x4D: return KDB_KEY_RIGHT;
				default:   continue;
			}
		}

		if (kdb_keys[sc])
			return kdb_keys[sc];
	}
}

static void kdb_readline(char* buf, int max) {
	int i = 0;
	int hpos = -1;                 /* -1 = editing a fresh line */

	for (;;) {
		int c = kdb_getchar();

		if (c == '\n') {
			buf[i] = 0;
			printf("\n");
			hist_add(buf);
			return;
		}

		if (c == '\b') {
			if (i) { i--; printf("\b"); }
			continue;
		}

		if (c == KDB_KEY_UP || c == KDB_KEY_DOWN) {
			if (!hist_count)
				continue;

			if (c == KDB_KEY_UP) {
				if (hpos >= hist_count - 1) continue;
				hpos++;
			} else {
				if (hpos < 0) continue;
				hpos--;
			}

			while (i > 0) { printf("\b"); i--; }   /* erase current line */

			if (hpos >= 0) {
				const char* h = hist_get(hpos);
				strcpy(buf, h);
				i = strlen(h);
				printf("%s", h);
			}
			continue;
		}

		if (c < 128 && i < max - 1) {
			buf[i++] = c;
			printf("%c", c);
		}
	}
}

/* Parse a hex value, with or without a 0x prefix */
static uint32_t parse_hex(const char* s) {
	uint32_t v = 0;
	if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;

	while (*s) {
		v <<= 4;
		if      (*s >= '0' && *s <= '9') v |= *s - '0';
		else if (*s >= 'a' && *s <= 'f') v |= *s - 'a' + 10;
		else if (*s >= 'A' && *s <= 'F') v |= *s - 'A' + 10;
		else break;
		s++;
	}
	return v;
}

static void cmd_help(void) {
	printf("Commands:\n");
	printf("  r          registers\n");
	printf("  bt         backtrace\n");
	printf("  x <addr>   hexdump 128 bytes\n");
	printf("  sym <addr> resolve address to symbol\n");
	printf("  v <addr>   translate virtual to physical\n");
	printf("  mem        memory statistics\n");
	printf("  tasks      list all tasks\n");
	printf("  ls         list initrd files\n");
	printf("  cat <name> print a file\n");
	printf("  history    shows history of commands\n");
	printf("  clear      clear the screen\n");
	printf("  c          continue\n");
	printf("  reboot     triple-fault reset\n");
}

static void cmd_tasks(void) {
	task_t* cur = task_current();
	task_t* t   = cur;
	int i = 0;

	do {
		uint32_t off;
		const char* n = symbol_lookup(t->regs.eip, &off);

		printf("  [%d]%s eip=%x esp=%x %s\n",
		       i++, (t == cur) ? " *" : "  ",
		       t->regs.eip, t->regs.esp, n ? n : "?");

		if (t != cur)
			stack_trace_from(t->regs.ebp, 5);

		t = t->next;
	} while (t != cur);
}

static void cmd_ls(void) {
	struct dirent* d;
	for (uint32_t i = 0; (d = vfs_readdir(fs_root, i)); i++) {
		struct fs_node* n = vfs_finddir(fs_root, d->name);
		printf("  %s  (%d bytes)\n", d->name, n ? n->length : 0);
	}
}

static void cmd_cat(const char* name) {
	struct fs_node* n = vfs_finddir(fs_root, name);
	if (!n) { printf("not found: %s\n", name); return; }

	uint8_t buf[128];
	uint32_t off = 0;

	while (off < n->length) {
		uint32_t got = vfs_read(n, off, sizeof(buf) - 1, buf);
		if (!got) break;
		buf[got] = 0;
		printf("%s", (char*) buf);
		off += got;
	}
	printf("\n");
}

void kdb_enter(struct registers* regs) {
	terminal_set_anchor();
	char line[80];

	printf("\n=== kernel debugger === (type 'help')\n");

	for (;;) {
		printf("kdb> ");
		kdb_readline(line, sizeof(line));

		char* arg = strchr(line, ' ');
		if (arg) *arg++ = 0;

		if (!strcmp(line, "help"))       cmd_help();
		else if (!strcmp(line, "r"))     { if (regs) dump_registers(regs);
		                                   else printf("no saved state\n"); }
		else if (!strcmp(line, "bt"))    stack_trace(16);
		else if (!strcmp(line, "x"))     { if (arg) hexdump(parse_hex(arg), 128); }
		else if (!strcmp(line, "sym"))   {
			uint32_t off;
			const char* n = symbol_lookup(parse_hex(arg), &off);
			printf(n ? "%s+0x%x\n" : "<unknown>\n", n, off);
		}
		else if (!strcmp(line, "v"))     {
			uint32_t p = paging_virt_to_phys(parse_hex(arg));
			printf("phys = 0x%x\n", p);
		}
		else if (!strcmp(line, "mem"))   {
			printf("free frames: %d\n", pmm_free_frame_count());
			printf("heap used:   %d bytes\n", kheap_used());
		}
		else if (!strcmp(line, "tasks")) cmd_tasks();
		else if (!strcmp(line, "ls"))  cmd_ls();
		else if (!strcmp(line, "cat")) { if (arg) cmd_cat(arg); }
		else if (!strcmp(line, "history")) {
			for (int k = hist_count - 1; k >= 0; k--)
				printf("  %d  %s\n", hist_count - k, hist_get(k));
		}
		else if (!strcmp(line, "clear")) terminal_clear_to_anchor();
		else if (!strcmp(line, "c"))     return;
		else if (!strcmp(line, "reboot")) {
			struct { uint16_t l; uint32_t b; } __attribute__((packed)) z = {0,0};
			__asm__ volatile ("lidt %0; int3" :: "m"(z));
		}
		else if (line[0])                printf("unknown: %s\n", line);
	}
}
