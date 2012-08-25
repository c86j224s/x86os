#include <kernel/vfs.h>

void init_vfs() {
	fs_init_fat16();
}
