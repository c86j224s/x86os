#ifndef __DRIVERS_TTY_H__
#define __DRIVERS_TTY_H__

#include <kernel/device.h>

void init_device_tty(void);
void close_device_tty(void);

int device_tty_open(int);
int device_tty_close(int);
int device_tty_read(int, void*, int);
int device_tty_write(int, void*, unsigned int);

enum {
	TTY_COMMAND_GET_CONF = 0,
	TTY_COMMAND_SET_CONF = 1,
	TTY_COMMAND_RELOCATE_CURSOR = 2
};
struct tty_conf_kt {
	int x, y;
	int bgcolor, fgcolor;
	int tabswidth;
} __attribute ((__packed__));
int device_tty_ioctl(int, unsigned int, void*);
int device_tty_lseek(int, int, int);

#endif
