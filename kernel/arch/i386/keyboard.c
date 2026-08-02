#include <stdio.h>
#include <kernel/io.h>
#include <kernel/irq.h>
#include <stdbool.h>

static const char scancode_ascii[128] = {
	0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
	'\t', 'q','w','e','r','t','y','u','i','o','p','[',']','\n',
	0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
	0, '\\', 'z','x','c','v','b','n','m',',','.','/', 0,
	'*', 0, ' ',
	/* rest zero */
};

static const char scancode_shifted[128] = {
	0, 27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
	'\t', 'Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
	0, 'A','S','D','F','G','H','J','K','L',':','"','~',
	0, '|', 'Z','X','C','V','B','N','M','<','>','?', 0,
	'*', 0, ' ',
};

static bool shift_down = false;
static bool caps_lock  = false;

static void keyboard_callback(struct registers* regs) {
	(void) regs;
	uint8_t sc = inb(0x60);

	/* Key release */
	if (sc & 0x80) {
		uint8_t released = sc & 0x7F;
		if (released == 0x2A || released == 0x36)
			shift_down = false;
		return;
	}

	/* Modifiers */
	if (sc == 0x2A || sc == 0x36) { shift_down = true; return; }
	if (sc == 0x3A) { caps_lock = !caps_lock; return; }

	char c = shift_down ? scancode_shifted[sc] : scancode_ascii[sc];

	/* Caps Lock affects letters only */
	if (caps_lock && c >= 'a' && c <= 'z') c -= 32;
	else if (caps_lock && shift_down && c >= 'A' && c <= 'Z') c += 32;

	if (c)
		printf("%c", c);
}

void keyboard_initialize(void) {
	irq_install_handler(1, keyboard_callback);
}
