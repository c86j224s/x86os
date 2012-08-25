#include <kernel/type.h>

#ifndef __PIO_H__
#define __PIO_H__

unsigned char inb(unsigned short);
void outb(unsigned short, unsigned char);

unsigned short inw(unsigned short);
void outw(unsigned short, unsigned short);

#endif
