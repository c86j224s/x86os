#include <boot/global.h>

void check_mem_size() {
	unsigned int *mem_pos = 0, mem_value;
	
	do {
		mem_pos += 0x100000;
		*mem_pos = 0x12345678;
		mem_value = *mem_pos;
	} while (mem_value == 0x12345678);
	
	*k_mem_size = (unsigned int)mem_pos;
}

void memset(unsigned char *offset, unsigned char value, int length) {
	while (length--) {
		*offset++ = value;
	}
}

void memcpy(byte *dest, byte *src, dword size) {
	while(size--) {
		*dest++ = *src++;
	}
}
