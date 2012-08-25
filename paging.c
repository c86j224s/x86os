#include <kernel/task.h>
#include <kernel/paging.h>

void* alloc_kernel_pages(int);
void* alloc_user_pages(int);
void free_kernel_pages(void*, int);
void free_user_pages(void*, int);

dword get_physical_kernel_address(dword);
dword get_virtual_kernel_address(dword);

dword *page_dir_table = PAGE_DIR_OFFSET;
dword *page_entry_table = PAGE_TBL_OFFSET;

void init_paging(void) {
	dword offset;
	int i;

	for(i = 0; i < 0x400; i++)
		page_dir_table[i] = PGD_EMPTY;
	for(i = 0; i < 0x400 * 0x400; i++)
		page_entry_table[i] = PGT_EMPTY;

	for(offset = 0; offset <= KERN_HEAP_OFFSET; offset += 4096)
		alloc_page(0);
	
	asm (
		"mov %0, %%eax;"
		"mov %%eax, %%cr3;"
		"mov %%cr0, %%eax;"
		"or $0x80000000, %%eax;"
		"mov %%eax, %%cr0;"
		:
		: "m"(page_dir_table)
	); 

}

void* alloc_page(int priv) {
	return alloc_multiple_pages(priv, 1);
}

void *alloc_multiple_pages(int priv, int count) {
	dword dir, ent;
	int cnt;

	if (priv == 0) {
		return alloc_kernel_pages(count);
	} else {
		return alloc_user_pages(count);
		/*
		ent = 0x100 * 0x400;
		while(1) {
			for(; page_entry_table[ent] != PGT_EMPTY; ent++);
			if (ent < 0x100 * 0x400 || ent == 0x400 * 0x400) return 0;
		
			for(cnt = 0; cnt < count && page_entry_table[ent+cnt] == PGT_EMPTY; cnt++);
			if (ent+cnt <= 0x100 * 0x400 || ent+cnt == 0x400 * 0x400) continue;
			if (cnt == count)
				break;
		}

		dir = (ent >> 10) & 0x3ff;

		// TODO 혹시 스케쥴링 때문에 체크하고 난 다음에 남은 메모리를 뺏기는 경우도 있지 않을까? 락 한번 걸어줘야 하지 않을까?
		if (physical_mem_remaining_count(count) < count)
			return 0;

		for(cnt = 0; cnt < count; cnt++) {
			dword paddr = physical_mem_alloc();
			if (paddr >= physical_mem_size)
				return 0;
			// TODO 여기서 그냥 안된다고 나가는게 아니라, 지금까지 했던 물리 할당을 롤백하거나, 애초에 처음부터 되는지 안되는지부터 검사해야 할 것 같다.
			page_entry_table[ent+cnt] = PGT_USER(paddr);
			page_dir_table[dir] = PGD_USER((dword)&page_entry_table[(ent+cnt) / 1024 * 1024]);
		}

		return (void*)(ent * 4096);
		*/
	}

	return 0;		
}

void free_page(void *address) {
	free_multiple_pages(address, 1);
}

void free_multiple_pages(void *address, int count) {
	if (address < 0x100 * 0x400 * 0x400) {
		free_kernel_pages(address, count);
	} else {
		// do nothing;;;
		/*
		dword addr = address;
		if (count < 1)
			return;
		
		while (count--) {
			int ent = (addr >> 12) & 0xfffff;
			int dir = (addr >> 22) & 0x3ff;
			int i;
	
			physical_mem_free(page_entry_table[ent]);
			page_entry_table[ent] = PGT_EMPTY;
			for(i = ent; i < ent+1024 && page_entry_table[i] == PGT_EMPTY; i++);
			if (i == ent+1024)
				page_dir_table[dir] = PGD_EMPTY;
	
			addr += 4096;
		}
		*/
	}
}

void* alloc_kernel_pages(int count) {
	dword dir, ent;
	int cnt;
	
	ent = 0;
	while(1) {
		for(; page_entry_table[ent] != PGT_EMPTY; ent++);
		if (ent >= 0x100 * 0x400) return 0;

		for(cnt = 0; cnt < count && page_entry_table[ent+cnt] == PGT_EMPTY; cnt++);
		if (ent+cnt >= 0x100 * 0x400) continue;
		if (cnt == count)
			break;
	}

	dir = (ent >> 10) & 0x3ff;
	
	// TODO 혹시 스케줄링 때문에 체크하고 난 다음에 남은 메모리를 뺏기는 경우도 있지 않을까? 락 한번 걸어줘야 하지 않을까?
	
	if (physical_mem_remaining_count(count) < count)
		return 0;
	
	for(cnt = 0; cnt < count; cnt++) {
		dword paddr = physical_mem_alloc();
		if (paddr >= physical_mem_size)
			return 0;
		page_entry_table[ent+cnt] = PGT_KERNEL(paddr);
		// 페이지 디렉토리가 가리키는 주소는 논리주소여야 하나, 맨 앞 5MB는 논리주소와 물리주소가 1:1 매핑이므로 편의상 이렇게 처리한다. -_-;
		page_dir_table[dir] = PGD_KERNEL((dword)&page_entry_table[(ent) / 1024 * 1024]);
	}
	
	return (void*)(ent * 4096);
}

void free_kernel_pages(void *address, int count) {
	dword addr = address;

	if (count < 1)
		return;

	while (count--) {
		int ent = (addr >> 12) & 0xffffff;
		int dir = (addr >> 22) & 0x3ff;
		int i;

		physical_mem_free(page_entry_table[ent]);
		page_entry_table[ent] = PGT_EMPTY;
		for(i = ent; i < ent+1024 && page_entry_table[i] == PGT_EMPTY; i++);
		if (i == ent+1024)
			page_dir_table[dir] = PGD_EMPTY;

		addr += 4096;
	}
}

void* alloc_user_pages(int count) {
	if (current_task == 0)
		return 0;
	
	dword *page_table = 0;
	int d, t, sd, st;
	int cnt;

	sd = st = 0;
	cnt = 0;
	for(d = 256; d < 1024; d++) {
		if (page_dir_table[d] == PGD_EMPTY) {
			page_table = alloc_kernel_pages(1);
			//page_dir_table[d] = PGD_KERNEL(get_physical_kernel_address(page_table));
			page_dir_table[d] = PGD_USER(get_physical_kernel_address(page_table));
		} else {
			page_table = get_virtual_kernel_address(page_dir_table[d]);
		}

		for (t = 0; t < 1024; t++) {
			if (page_table[t] == 0) {
				if (cnt == 0) {	
					sd = d;
					st = t;
				}
				cnt ++;
			} else {
				cnt = 0;
			}

			if (cnt == count)
				goto found;
		}
	}
	return 0;

found: 
	for (d = sd, cnt = 0; d < 1024; d++) {
		page_table = get_virtual_kernel_address(page_dir_table[d]);

		for(t = (d==sd? st:0) ; t < 1024;  t++) {
			if (cnt == count)
				break;
			
			page_table[t] = PGT_USER(physical_mem_alloc());
			cnt++;
		}
		if (cnt == count)
			break;
	}

	return (sd<<22) + (st<<12);
}

void free_user_pages(void *address, int count) {
}

dword get_physical_kernel_address(dword vaddr) {
	return page_entry_table[(vaddr>>12) & 0xfffff] & 0xfffff000;
}

dword get_virtual_kernel_address(dword paddr) {
	int ent;
	for(ent = 0; ent < 0x100 * 0x400; ent ++) {
		if (page_entry_table[ent] >> 12 == paddr >> 12)
			return ent<<12;
	}
	return 0;
}
