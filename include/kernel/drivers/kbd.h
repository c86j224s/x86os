#ifndef __DRIVERS_KBD_H__
#define __DRIVERS_KBD_H__

#include <kernel/device.h>

void init_device_kbd(void);
void close_device_kbd(void);

int device_kbd_open(int);
int device_kbd_close(int);
int device_kbd_read(int, void*, int);
int device_kbd_write(int, void*, unsigned int);
int device_kbd_ioctl(int, unsigned int, void*);
int device_kbd_lseek(int, int, int);

void device_kbd_interrupt(void);

#endif
