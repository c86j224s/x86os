#include <kernel/task.h>
#include <kernel/syscall.h>
#include <kernel/device.h>
#include <kernel/drivers/tty.h>
#include <kernel/drivers/kbd.h>

void* syscall_table[256] = {
	syscallPutChar,
	syscallPuts,
	syscallInitKbd,
	syscallReleaseKbd,
	syscallHasKey,
	syscallGetKey,
	syscallGetCurX,
	syscallGetCurY,
	syscallMoveXY,
	syscallDrawChar,
	0,
};

int syscallPutChar(int c) {
	return k_putchar(c);
}

int syscallPuts(char *s) {
	return k_puts(s);
}

int syscallInitKbd() {
	return device_open(KBD, 0);
}

int syscallReleaseKbd() {
	return device_close(KBD, 0);
}

int syscallHasKey() {
	return device_lseek(KBD, 0, 0, SEEK_CUR);
}

int syscallGetKey() {
	char c;
	if (device_read(KBD, 0, &c, 1) > 0)
		return c;
	return c;
}

int syscallGetCurX() {
	struct tty_conf_kt t;
	device_ioctl(TTY, 0, TTY_COMMAND_GET_CONF, &t);
	return t.x;
}

int syscallGetCurY() {
	struct tty_conf_kt t;
	device_ioctl(TTY, 0, TTY_COMMAND_GET_CONF, &t);
	return t.y;
}

void syscallMoveXY(int x, int y) {
	struct tty_conf_kt t;
	device_ioctl(TTY, 0, TTY_COMMAND_GET_CONF, &t);
	t.x = x;
	t.y = y;
	device_ioctl(TTY, 0, TTY_COMMAND_SET_CONF, &t);
}

int syscallDrawChar(int c) {
	return k_drawchar(c);
}

/*
int syscallDevOpen(int major, int minor) {
	if (major <= 0 || minor < 0)
		return -1;
	if (device_open(major, minor) == -1)
		return -1;
	current_task->descs[current_task->desc_count].major = major;
	current_task->descs[current_task->desc_count].minor = minor;
	current_task->desc_count++;
	return current_task->desc_count-1;
}

int syscallDevClose(int des) {
	if (des < 0 ||
		(device_close(
			current_task->descs[des].major, 
			current_task->descs[des].minor) == -1))
		return -1;
	current_task->descs[des].major = 0;
	current_task->descs[des].minor = 0;
	while (current_task->descs[current_task->desc_count-1].major == 0)
		current_task->desc_count--;
	return 0;
}

int syscallDevRead(int des, void *target, int length) {
	return device_read(current_task->descs[des].major,
		current_task->descs[des].minor,
		target, length);
}

int syscallDevWrite(int des, void *source, int length) {
	return device_write(current_task->descs[des].major,
		current_task->descs[des].minor,
		source, length);
}

int syscallDevIoctl(int des, unsigned int command, void *value) {
	return device_ioctl(current_task->descs[des].major,
		current_task->descs[des].minor,
		command, value);
}

int syscallDevLseek(int des, int offset, int start) {
	return device_lseek(current_task->descs[des].major,
		current_task->descs[des].minor,
		offset, start);
}
*/
