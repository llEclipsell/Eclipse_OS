#include <kernel/io.h>
#include <kernel/serial.h>

#define COM1 0x3F8

void serial_initialize(void) {
	outb(COM1 + 1, 0x00);   /* disable interrupts */
	outb(COM1 + 3, 0x80);   /* enable DLAB (set baud divisor) */
	outb(COM1 + 0, 0x03);   /* divisor low  = 3 → 38400 baud */
	outb(COM1 + 1, 0x00);   /* divisor high = 0 */
	outb(COM1 + 3, 0x03);   /* 8 bits, no parity, one stop bit */
	outb(COM1 + 2, 0xC7);   /* enable FIFO, clear, 14-byte threshold */
	outb(COM1 + 4, 0x0B);   /* IRQs enabled, RTS/DSR set */
}

static bool transmit_empty(void) {
	return inb(COM1 + 5) & 0x20;
}

void serial_putchar(char c) {
	if (c == '\n')
		serial_putchar('\r');       /* terminals want CRLF */
	while (!transmit_empty())
		;
	outb(COM1, c);
}

void serial_write(const char* s) {
	while (*s)
		serial_putchar(*s++);
}

bool serial_available(void) {
	return inb(COM1 + 5) & 1;
}

char serial_getchar(void) {
	while (!serial_available())
		;
	return inb(COM1);
}
