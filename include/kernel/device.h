#ifndef __DEVICE_H__
#define __DEVICE_H__

#define DEVICE_MAX_MAJOR 256

struct device_operations {
	int (*open)(int minor);
	int (*close)(int minor);
	int (*read)(int minor, void *target, int length);
	int (*write)(int minor, void *source, int length);
	int (*ioctl)(int minor, unsigned int command, void* value);
	int (*lseek)(int minor, int offset, int start);
};

struct device_kt {
	char device_name[64];
	struct device_operations operations;
	int allocated;
	int use;
} __attribute__((__packed__));
extern struct device_kt devices[];
extern int devices_count;
#define TTY 1
#define KBD 2
#define IDE 3
#define FAT16FS 4

void init_device_drivers(void);
struct device_kt* get_major_device(int major);

struct device_descriptor_kt {
	unsigned char major;
	unsigned char minor;
	unsigned char reserved1;
	unsigned char reserved2;
} __attribute__ ((__packed__));
extern struct device_descriptor_kt dev_desc[];
extern int dev_desc_count;

int device_open(int major, int minor);
int device_close(int major, int minor);
int device_read(int major, int minor, void* target, int length);
int device_write(int major, int minor, void* source, int length);
int device_ioctl(int major, int minor, unsigned int command, void* value);
int device_lseek(int major, int minor, int offset, int start);

enum {
	SEEK_SET = 0,
	SEEK_CUR = 1,
	SEEK_END = 2
};

#endif
