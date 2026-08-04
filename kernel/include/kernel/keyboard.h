#ifndef _KERNEL_KEYBOARD_H
#define _KERNEL_KEYBOARD_H

#include <stdbool.h>

void keyboard_initialize(void);

bool keyboard_available(void);	/* is there a charater waiting */
char keyboard_getchar(void);	/* blocks until one arrives */

#endif
