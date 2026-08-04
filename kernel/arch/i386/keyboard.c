#include <stdio.h>
#include <kernel/io.h>
#include <kernel/irq.h>
#include <stdbool.h>

#define KBD_BUFFER_SIZE 128

static char     kbd_buffer[KBD_BUFFER_SIZE];
static volatile uint32_t kbd_head = 0;   /* written by the IRQ */
static volatile uint32_t kbd_tail = 0;   /* read by normal code */

static void kbd_push(char c) {
	uint32_t next = (kbd_head + 1) % KBD_BUFFER_SIZE;
	if (next == kbd_tail)
		return;                          /* full — drop the key */
	kbd_buffer[kbd_head] = c;
	kbd_head = next;
}

bool keyboard_available(void) {
	return kbd_head != kbd_tail;
}

char keyboard_getchar(void) {
	while (kbd_head == kbd_tail)
		__asm__ volatile ("hlt");        /* sleep until an IRQ arrives */

	char c = kbd_buffer[kbd_tail];
	kbd_tail = (kbd_tail + 1) % KBD_BUFFER_SIZE;
	return c;
}

#define KBD_DATA   0x60
#define KBD_STATUS 0x64

static void kbd_wait_input(void) {
	while (inb(KBD_STATUS) & 0x2)        /* bit 1 = input buffer full */
		;
}

static void kbd_send(uint8_t byte) {
	kbd_wait_input();
	outb(KBD_DATA, byte);
}

static void kbd_set_leds(bool scroll, bool num, bool caps) {
	kbd_send(0xED);
	kbd_send((scroll ? 1 : 0) | (num ? 2 : 0) | (caps ? 4 : 0));
}

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
static bool ctrl_down = false;
static bool alt_down  = false;
static bool extended = false;

static void keyboard_callback(struct registers* regs) {
	(void) regs;
	uint8_t sc = inb(0x60);

	if (sc == 0xE0) {
		extended = true;
		return;
	}

	if (extended) {
		extended = false;

		if (sc & 0x80) {                 /* extended key release */
			if (sc == 0x9D) ctrl_down = false;
			if (sc == 0xB8) alt_down  = false;
			return;
		}

		switch (sc) {
			case 0x1D: ctrl_down = true; break;   /* right Ctrl */
			case 0x38: alt_down  = true; break;   /* right Alt  */
			case 0x48: /* up    */ break;
			case 0x50: /* down  */ break;
			case 0x4B: /* left  */ break;
			case 0x4D: /* right */ break;
			case 0x47: /* home  */ break;
			case 0x4F: /* end   */ break;
			case 0x53: /* delete */ break;
		}
		return;
	}

	/* Key release */
	if (sc & 0x80) {
		uint8_t released = sc & 0x7F;
		if (released == 0x2A || released == 0x36)
			shift_down = false;
		return;
	}

	/* Modifiers */
	if (sc == 0x2A || sc == 0x36) { shift_down = true; return; }
	if (sc == 0x3A) {
		caps_lock = !caps_lock;
		kbd_set_leds(false, false, caps_lock);
		return;
	}

	char c = shift_down ? scancode_shifted[sc] : scancode_ascii[sc];

	/* Caps Lock affects letters only */
	if (caps_lock && c >= 'a' && c <= 'z') c -= 32;
	else if (caps_lock && shift_down && c >= 'A' && c <= 'Z') c += 32;

	if (c)
		kbd_push(c);
}

void keyboard_initialize(void) {
	irq_install_handler(1, keyboard_callback);
}
