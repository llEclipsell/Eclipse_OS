#include <kernel/io.h>
#include <kernel/pic.h>

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI   0x20

void pic_remap(int offset1, int offset2) {
	uint8_t mask1 = inb(PIC1_DATA);
	uint8_t mask2 = inb(PIC2_DATA);

	outb(PIC1_CMD, 0x11); io_wait();   /* ICW1: begin init */
	outb(PIC2_CMD, 0x11); io_wait();

	outb(PIC1_DATA, offset1); io_wait();  /* ICW2: vector offset */
	outb(PIC2_DATA, offset2); io_wait();

	outb(PIC1_DATA, 0x04); io_wait();  /* ICW3: slave on IRQ2 */
	outb(PIC2_DATA, 0x02); io_wait();  /* ICW3: slave identity */

	outb(PIC1_DATA, 0x01); io_wait();  /* ICW4: 8086 mode */
	outb(PIC2_DATA, 0x01); io_wait();

	outb(PIC1_DATA, mask1);
	outb(PIC2_DATA, mask2);
}

void pic_send_eoi(uint8_t irq) {
	if (irq >= 8)
		outb(PIC2_CMD, PIC_EOI);
	outb(PIC1_CMD, PIC_EOI);
}

void pic_set_mask(uint8_t irq) {
	uint16_t port = irq < 8 ? PIC1_DATA : PIC2_DATA;
	if (irq >= 8) irq -= 8;
	outb(port, inb(port) | (1 << irq));
}

void pic_clear_mask(uint8_t irq) {
	uint16_t port = irq < 8 ? PIC1_DATA : PIC2_DATA;
	if (irq >= 8) irq -= 8;
	outb(port, inb(port) & ~(1 << irq));
}
