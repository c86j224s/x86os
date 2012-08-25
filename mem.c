#include <kernel/mem.h>

byte physical_mem_table[1024*1024] = {0,};
dword physical_mem_size = 0;

void check_physical_mem_size() {
	dword *mem_pos = 0, mem_value;
	
	do {
		mem_pos += 0x100000;
		*mem_pos = 0x12345678;
		mem_value = *mem_pos;
	} while (mem_value == 0x12345678);
										
	physical_mem_size = (dword)mem_pos;
}

dword physical_mem_alloc() {
	int i;
	for(i = 0; i < 1048576; i++) {
		if (physical_mem_table[i] < 1) {
			physical_mem_table[i]++;
			break;
		}
	}
	return i * 4096;
}

void physical_mem_free(dword address) {
	address /= 4096;
	if (physical_mem_table[address] > 0)
		physical_mem_table[address]--;
}

dword physical_mem_remaining_count(dword frames) {
	int i, j;
	for (i = 0, j = 0; i < 1048567; i++) {
		if (physical_mem_table[i] < 1) j++;
		if (frames > 0 && j == frames) break;
	}
	return j;
}
