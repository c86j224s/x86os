#include <boot/global.h>
#include <boot/memory.h>
#include <boot/ide.h>
#include <boot/scr.h>
#include <boot/fat16.h>

#define KERNEL_POSITION 0x500000

int load_kernel();
void halt(char *);

int main() {
	int ret;
	
	print_chars("\nentering kernel loader..\n");
	
	check_mem_size(); 
	if (*k_mem_size < 0x400000)
		halt("short of memory, at least up to 4MB.");
	
	ret = load_kernel();
	if (ret)
		halt("kernel not found.");
	
	print_chars("loaded kernel successfully.\n");
	print_chars("\n");
	
	asm(
		"ljmp $0x8, %0;"
		:
		: "i"(KERNEL_POSITION)
	); 
	
	return 0;
}

int load_kernel() {
	unsigned char kernel_name[11] = {'K', 'E', 'R', 'N', 'E', 'L', 0x20, 0x20, 0x20, 0x20, 0x20};

	dword cluster;
	read_boot_record();
	read_fats_and_root_directory_entry();
	if ((cluster = find_cluster(kernel_name, 0x20)) == 0)
		return -1;
	*k_kernel_size = read_cluster(cluster);
	memcpy(KERNEL_POSITION, position, *k_kernel_size * 512);

	return 0;
}

void halt(char *msg) {
	print_chars(msg);
	for(;;);
}

dword *k_mem_size = (dword *)0x100000;
dword *k_kernel_size = (dword *)0x100004;
