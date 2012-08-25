#ifndef __FAT16_H__
#define __FAT16_H__

#include <kernel/type.h>

struct fat16_br_kt {
	/* common informations */
	byte jump_boot_code[3];
	byte oem_name[8];
	word bytes_per_sector;
	byte sectors_per_cluster;
	word reserved_sector_count;
	byte number_of_fats;
	word root_directory_entry_count;
	word total_sector_16;
	byte media;
	word fat_size_16;
	word sectors_per_track;
	word number_of_heads;
	dword hidden_sector;
	dword total_sector_32;
	
	// fat 16 only
	byte drive_number;
	byte reserved1;
	byte boot_signature;
	dword volume_id;
	byte volume_label[11];
	byte file_system_type[8];
} __attribute__ ((__packed__));

extern struct fat16_br_kt *fat16_br;

extern word *fat16_fat;

struct fat16_dirent_kt {
	byte name[8];
	byte extender[3];
	byte attribute;
	byte nt_resource_reserved;
	byte create_time_tenth;
	word create_time;
	word create_date;
	word last_access_date;
	word first_cluster_high;
	word write_time;
	word write_date;
	word first_cluster_low;
	dword file_size;
} __attribute__ ((__packed__));

#define FAT16_DIRENT_ATTR_READONLY	0x01
#define FAT16_DIRENT_ATTR_HIDDEN	0x02
#define FAT16_DIRENT_ATTR_SYSTEM	0x04
#define FAT16_DIRENT_ATTR_VOLUME	0x08
#define FAT16_DIRENT_ATTR_DIRECTORY	0x10
#define FAT16_DIRENT_ATTR_ARCHIVE	0x20
#define FAT16_DIRENT_ATTR_LONGNAME1	0xF0
#define FAT16_DIRENT_ATTR_LONGNAME2 0x0F

extern struct fat16_dirent_kt *fat16_root;

void fs_init_fat16(void);

struct fat16_file_kt {
	struct fat16_dirent_kt information;
	int linear_offset;
	byte* buffer;
} __attribute__ ((__packed__));
struct fat16_file_kt fs_open_fat16(byte *);
void fs_close_fat16(struct fat16_file_kt);
int fs_read_fat16(struct fat16_file_kt, void*, int);
// TODO write, lseek, ioctl

#endif
