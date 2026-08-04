#include <stdio.h>
#include <string.h>
#include <kernel/io.h>
#include <kernel/kdb.h>
#include <kernel/debug.h>
#include <kernel/pmm.h>
#include <kernel/kheap.h>
#include <kernel/paging.h>

static const char kdb_keys[128] = {
	0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
	'\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
	0,'a','s','d','f','g','h','j','k','l',';','\'','`',
	0,'\\','z','x','c','v','b','n','m',',','.','/', 0, '*', 0, ' ',
};

static char kdb_getchar(void) {
	for (;;) {
		if (inb(0x64) & 1) {                 /* output buffer full */
			uint8_t sc = inb(0x60);
			if (!(sc & 0x80) && kdb_keys[sc])
				return kdb_keys[sc];
		}
	}
}

static void kdb_readline(char* buf, int max) {
	int i = 0;
	for (;;) {
		char c = kdb_getchar();

		if (c == '\n') { buf[i] = 0; printf("\n"); return; }
		if (c == '\b') { if (i) { i--; printf("\b"); } continue; }
		if (i < max - 1) { buf[i++] = c; printf("%c", c); }
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
	printf("  c          continue\n");
	printf("  reboot     triple-fault reset\n");
}

void kdb_enter(struct registers* regs) {
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
		else if (!strcmp(line, "c"))     return;
		else if (!strcmp(line, "reboot")) {
			struct { uint16_t l; uint32_t b; } __attribute__((packed)) z = {0,0};
			__asm__ volatile ("lidt %0; int3" :: "m"(z));
		}
		else if (line[0])                printf("unknown: %s\n", line);
	}
}
