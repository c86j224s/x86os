#include <kernel/fs/fat16.h>
#include <kernel/device.h>
#include <kernel/paging.h>
#include <kernel/util.h>

/* -------------------- 내부 구조체와 함수들 -------------------- */
byte br_space[512] = {0,};
struct fat16_br_kt * fat16_br = 0;
void read_boot_record(void);

word *fat16_fat;
void read_fat(void);

struct fat16_dirent_kt *fat16_root;
void read_root_directory_entry(void);

struct fat16_dirent_kt find_cluster (byte *, struct fat16_dirent_kt *);
word next_cluster (word);
word cluster_offset(word);
int read_cluster(word, byte *);

int child_node(char *, char*);
void dos16_name(char *, char *, char *);

/* -------------------- 외부에 보여지는 인터페이스 -------------------- */
void fs_init_fat16 (void) {
	read_boot_record();
	read_fat();
	read_root_directory_entry();
}

byte dirents[512*32];
struct fat16_file_kt fs_open_fat16(byte *path) {
	// TODO 들어온 경로를 절대경로로 바꿔야한다. 현재는 무조건 절대경로라고 가정.
	char node[256], dosname[11];
	struct fat16_dirent_kt dent = {0, };
	struct fat16_file_kt file = {0, };
	byte *dirs = fat16_root;

	while (1) {
		int cluster, k;

		k = child_node(path, node);
		if (k == 0) break;
		path += k;
		dos16_name(node, dosname, dosname+8);
		dent = find_cluster(dosname, dirs);
		if (dent.first_cluster_low == 0) break;
		if (*path == 0) {	// 파일이다.
			file.information = dent;
			file.linear_offset = 0;
			file.buffer = alloc_multiple_pages(0, file.information.file_size/ 4096 + (file.information.file_size % 4096 > 0));
			read_cluster(file.information.first_cluster_low, file.buffer);
			break;
		} else {			// 디렉토리다.
			read_cluster(dent.first_cluster_low, dirents);
			dirs = dirents;
		}
	}
	return file;
}

void fs_close_fat16(struct fat16_file_kt p_file) {
	// TODO 페이지를 해제하려고 할 때 문제가 발생한다. 아무래도 페이지 해제 루틴 자체의 문제인 거 같은데, 원인을 모르겠다. 해제하고 난 후, 복귀할 때 문제가 생긴다.
	free_multiple_pages(p_file.buffer, p_file.information.file_size/4096 + (p_file.information.file_size%4096>0));
}

int fs_read_fat16(struct fat16_file_kt file, void *target, int size) {
	int cnt = 0;
	if (file.linear_offset+size < file.information.file_size)
		cnt = size;
	else
		cnt = file.information.file_size - file.linear_offset;
	memcpy(target, ((byte*)file.buffer + file.linear_offset), cnt);
	file.linear_offset += cnt;
	return cnt;
}

/* -------------------- 내부에서 사용하는 함수들 -------------------- */
void read_boot_record() {
	device_open(IDE, 0);
	device_read(IDE, 0, br_space, 1);
	device_close(IDE, 0);
	fat16_br = br_space;
}

void read_fat() {
	word *fp;
	int i;
	fp = alloc_page(PRIV_KERN);
	fat16_fat = fp;
	for(i = 1; i < fat16_br->fat_size_16/4 + fat16_br->fat_size_16%4; i++) {
		fp = alloc_page(PRIV_KERN);	// TODO 메모리 공간이 연속해서 할당된다는 것이 보장되어야 한다!! (alloc multiple pages 구현!)
	}

	device_open(IDE, 0);
	device_lseek(IDE, 0, fat16_br->reserved_sector_count, SEEK_SET);
	device_read(IDE, 0, fat16_fat, fat16_br->fat_size_16);
	device_close(IDE, 0);
}

void read_root_directory_entry() {
	byte *fp;
	int root_dir_pages, root_dir_offset, root_dir_sectors;
	int i;

	root_dir_pages = fat16_br->root_directory_entry_count*32/4096 + fat16_br->root_directory_entry_count*32%4096;
	root_dir_offset = fat16_br->reserved_sector_count + fat16_br->fat_size_16 * fat16_br->number_of_fats;
	root_dir_sectors = fat16_br->root_directory_entry_count*32/512 + fat16_br->root_directory_entry_count*32%512;

	fp = alloc_page(PRIV_KERN);
	fat16_root = fp;
	for(i = 1; i < root_dir_pages; i++) {
		fp = alloc_page(PRIV_KERN);
	}

	device_open(IDE, 0);
	device_lseek(IDE, 0, root_dir_offset, SEEK_SET);
	device_read(IDE, 0, fat16_root, root_dir_sectors);
	device_close(IDE, 0);
}

struct fat16_dirent_kt find_cluster (byte *nodename, struct fat16_dirent_kt *position) {
	struct fat16_dirent_kt *temp = position;
	struct fat16_dirent_kt err_ret = {0, };
	int i, j;

	for(i = 0; i < 512; i++) {
		if (temp[i].name[0] == 0xE5);
		else if (temp[i].name[0] == 0xF5 || temp[i].name[0] == 0x0)
			break;
		else {
			for(j = 0; j < 8 && temp[i].name[j] == nodename[j]; j++);
			if (j != 8) continue;
			for(j = 0; j < 3 && temp[i].extender[j] == nodename[8+j]; j++);
			if (j != 3) continue;

			return temp[i];
		}
	}

	return err_ret;
}

word next_cluster (word cluster) {
	return *(fat16_fat + cluster);
}

word cluster_offset(word cluster) {
	int data_offset = fat16_br->reserved_sector_count + fat16_br->fat_size_16 * fat16_br->number_of_fats + (fat16_br->root_directory_entry_count * 32 / fat16_br->bytes_per_sector);
	return data_offset + (cluster - 2) * fat16_br->sectors_per_cluster;
}

// target에 메모리가 미리 할당되어 있어야 할 것 같다..
int read_cluster(word starting_cluster, byte *target) {
	int i = 0;
	device_open(IDE, 0);
	do {
		device_lseek(IDE, 0, cluster_offset(starting_cluster), SEEK_SET);
		device_read(IDE, 0, target + i * fat16_br->sectors_per_cluster * fat16_br->bytes_per_sector, fat16_br->sectors_per_cluster);
		starting_cluster = next_cluster(starting_cluster);
		i++;
	} while (starting_cluster < 0xfff0);
	return i;
}

int child_node(char *path, char *node) {
	int i = 0;
	if (*path == '/') { path++; i++; }
	for(; *path && *path != '/'; *node++ = *path++, i++);
	*node = 0;
	return i;
}

void dos16_name(char *node, char *name, char *ext) {
	int i;
	for(i = 0; i < 8; i++) name[i] = ' ';
	for(i = 0; i < 3; i++) ext[i] = ' ';

	for(i = 0; i < 8 && *node && *node != '.'; *name++ = toupper(*node++), i++);

	for(; *node != '.'; node++) if (*node == 0) return;
	node++;

	for(i = 0; i < 3 && *node; *ext++ = toupper(*node++), i++);
}
