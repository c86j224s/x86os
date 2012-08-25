#include <kernel/pio.h>

unsigned char inb(unsigned short port) {
	unsigned char val = 0;
	asm (
		"mov %1, %%dx;"
		"inb %%dx, %%al;"
		"mov %%al, %0;"
		: "=m"(val)
		: "m"(port)
	);
	return val;
}

void outb(unsigned short port, unsigned char val) {
	asm (
		"mov %0, %%dx;"
		"mov %1, %%al;"
		"outb %%al, %%dx;"
		:
		: "m"(port),
		  "m"(val)
	);
}

unsigned short inw(unsigned short port) {
	unsigned short val = 0;
	asm (
		"mov %1, %%dx;"
		"inw %%dx, %%ax;"
		"mov %%ax, %0;"
		: "=m"(val)
		: "m"(port)
	);
	return val;
}

void outw(unsigned short port, unsigned short val) {
	asm(
		"mov %0, %%dx;"
		"mov %1, %%ax;"
		"outw %%ax, %%dx;"
		:
		: "m"(port),
		  "m"(val)
	);
}
