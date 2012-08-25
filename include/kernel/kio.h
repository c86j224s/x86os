#ifndef __KIO_H__
#define __KIO_H__

#include <kernel/varg.h>
#include <kernel/drivers/tty.h>

enum {
	TTY_BLACK = 0,
	TTY_BLUE = 1,
	TTY_GREEN = 2,
	TTY_CYAN = 3,
	TTY_RED = 4,
	TTY_MAGENTA = 5,
	TTY_BROWN = 6,
	TTY_LIGHTGRAY = 7,
	TTY_DARKGRAY = 8,
	TTY_LIGHTBLUE = 9,
	TTY_LIGHTGREEN = 10,
	TTY_LIGHTCYAN = 11,
	TTY_LIGHTRED = 12,
	TTY_LIGHTMAGENTA = 13,
	TTY_LIGHTBROWN = 14,
	TTY_WHITE = 15
};

int k_getxy(int *, int *);
int k_movexy(int, int);
int k_getcolor(int *, int *);
int k_setcolor(int, int);
int k_putchar(int);
int k_puts(char *);
int k_sprintf(char *, char *, ...);
int k_printf(char *, ...);

#endif
