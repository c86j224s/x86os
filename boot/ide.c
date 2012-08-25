#include <boot/io.h>
#include <boot/ide.h>

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
