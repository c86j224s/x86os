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

void read_sector_lba(struct chs, unsigned short *);
