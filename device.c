#include <kernel/device.h>
#include <kernel/drivers/tty.h>
#include <kernel/drivers/kbd.h>
#include <kernel/drivers/ide.h>

struct device_kt devices[DEVICE_MAX_MAJOR] = {0,};
int devices_count = 1;

void init_device_drivers() {
	init_device_tty();
	init_device_kbd();
	init_device_ide();
}

int device_open(int major, int minor) {
	if (! devices[major].allocated)
		return -1;
	
	return devices[major].operations.open(minor);
}

int device_close(int major, int minor) {
	if (! devices[major].allocated)
		return -1;

	return devices[major].operations.close(minor);
}

int device_read(int major, int minor, void *target, int length) {
	if (! devices[major].allocated)
		return -1;

	return devices[major].operations.read(minor, target, length);
}

int device_write(int major, int minor, void *source, int length) {
	if (! devices[major].allocated)
		return -1;
	
	return devices[major].operations.write(minor, source, length);
}

int device_ioctl(int major, int minor, unsigned int command, void* value) {
	if (! devices[major].allocated)
		return -1;
	
	return devices[major].operations.ioctl(minor, command, value);
}

int device_lseek(int major, int minor, int offset, int start) {
	if (! devices[major].allocated)
		return -1;
	
	return devices[major].operations.lseek(minor, offset, start);
}
