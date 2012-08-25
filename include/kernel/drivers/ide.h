#ifndef __DRIVERS_IDE_H__
#define __DRIVERS_IDE_H__

#include <kernel/device.h>

void init_device_ide(void);
void close_device_ide(void);

int device_ide_open(int);
int device_ide_close(int);
int device_ide_read(int, void*, int);
int device_ide_write(int, void*, unsigned int);
int device_ide_ioctl(int, unsigned int, void*);
int device_ide_lseek(int, int, int);

#endif
