#ifndef _KERNEL_SERIAL_H
#define _KERNEL_SERIAL_H

#include <stdbool.h>

void serial_initialize(void);
void serial_putchar(char c);
void serial_write(const char* s);
bool serial_available(void);
char serial_getchar(void);

#endif
