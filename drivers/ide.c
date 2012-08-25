#include <kernel/idt.h>
#include <kernel/device.h>
#include <kernel/drivers/ide.h>
#include <kernel/task.h>
#include <kernel/kio.h>
#include <kernel/util.h>

// 초대충 -_-;;;

int ide_sector = 0;
int ide_offset = 0;

void read_sector(int, int, unsigned char *);

void init_device_ide() {
	if (devices_count > IDE && devices[IDE].allocated)
		return;

	if (devices_count <= IDE)
		devices_count = IDE+1;
		
	devices[IDE].allocated = 1;
	k_sprintf(devices[IDE].device_name, "ide");
	devices[IDE].operations = (struct device_operations) {
		.open = device_ide_open,
		.close = device_ide_close,
		.read = device_ide_read,
		.write = device_ide_write,
		.ioctl = device_ide_ioctl,
		.lseek = device_ide_lseek
	};
	devices[IDE].use = 0;
	
	// 추가적인 하드디스크 드라이버 설정
	// 뭐가 있을까 ㅡㅡ;;
	// ide에 대한 정보?

	// 하드디스크 인터럽트 설정이 아마 여기 들어가야겠지?
}

void close_device_ide() {
	devices[IDE].allocated = 0;
}

int device_ide_open(int minor) {
	devices[IDE].use++;
	// 하드디스크가 존재하는지 체크해보자.
	return minor;
}

int device_ide_close(int minor) {
	devices[IDE].use--;
	return 0;
}

// sector 단위로 읽는다.
// TODO 락을 걸어서 두개의 프로세스가 동시에 진입하지 못하게 해야 한다.
int device_ide_read(int minor, void *target, int length) {
	unsigned char *trg = (unsigned char *)target;
	read_sector(ide_sector, length, trg);
	ide_sector += length;
	return length;
}

int device_ide_write(int minor, void *source, unsigned int length) {
	return -1;
}

int device_ide_ioctl(int minor, unsigned int command, void *value) {
	switch(command) {
		default:
			return -1;
	}
	return 0;
}

int device_ide_lseek(int minor, int offset, int start) {
	int off;
	
	switch(start) {
		case SEEK_SET:
			off = offset;
			if (off < 0)
				return -1;
			break;
		case SEEK_CUR:
			off = offset + ide_sector;
			if (off < 0)
				return -1;
			break;
		case SEEK_END:
			return -1;
			break;
	}

	ide_sector = off;	
	return 0;
}

// include/boot/ide.h, boot/ide.c와 동일하다..

#define PRIMARY_IDE_BASE	0x1f0
#define SECONDARY_IDE_BASE	0x170

#define IDE_DATA			0x00		// r/w
#define IDE_ERROR			0x01		// read only
#define IDE_SECTOR_COUNT	0x02		// r/w
#define IDE_SECTOR_NUMBER	0x03		// r/w
#define IDE_CYLINDER_LOW	0x04		// r/w
#define	IDE_CYLINDER_HIGH	0x05		// r/w
#define IDE_DRIVE_HEAD		0x06		// r/w
#define IDE_COMMAND			0x07		// write only
#define	IDE_STATUS			0x07		// read only

#define IDE_COMMAND_RECALIBRATE	0x10	// ??
#define IDE_COMMAND_READ		0x20	// with retry (without retry = 0x21)
#define	IDE_COMMAND_SEEK		0x70	// ??
#define IDE_COMMAND_IDENTIFY	0xEC	// ??

struct chs {
	unsigned char disk;
	unsigned char drive;
	unsigned int sector, cylinder, head;
	unsigned char count;
};

#define LBA2CHS(DISK,DRIVE,LBA,COUNT) \
	((struct chs) { \
		.disk = (DISK) & 0x01, \
		.drive = (DRIVE) & 0x01, \
		.sector = (LBA) & 0xff, \
		.cylinder = ((LBA)>>8) & 0xffff, \
		.head = ((LBA)>>24) & 0x0f, \
		.count = (COUNT) \
	})
	
void read_sector_lba(struct chs reader, unsigned short *buffer) {
	unsigned short base_port = (reader.disk & 1)? SECONDARY_IDE_BASE : PRIMARY_IDE_BASE;
	unsigned char temp = 0;
	int i;
	outb(base_port + IDE_SECTOR_COUNT, reader.count);

	outb(base_port + IDE_SECTOR_NUMBER, reader.sector);
	outb(base_port + IDE_CYLINDER_LOW, reader.cylinder & 0xff);
	outb(base_port + IDE_CYLINDER_HIGH, reader.cylinder >> 8);
	temp = 0xe0 | (reader.drive << 4) | (reader.head & 0x0f); 
	outb(base_port + IDE_DRIVE_HEAD, temp);

	outb(base_port + IDE_COMMAND, IDE_COMMAND_READ);
	do {
		*buffer = inb(base_port + IDE_STATUS);
	} while (! (*buffer & 0x08));

	for(i = 0; i < reader.count * 512 / 2; i++) {
		*buffer++ = inw(base_port + IDE_DATA);
	}
}

void read_sector(int sector, int count, unsigned char *buf) {
	unsigned short *sbuf = (unsigned short *)buf;
	struct chs reader = LBA2CHS(0, 0, sector, count);
	read_sector_lba(reader, sbuf);
}
