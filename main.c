#include <kernel/gdt.h>
#include <kernel/idt.h>	
#include <kernel/kio.h>
#include <kernel/addr.h>
#include <kernel/mem.h>
#include <kernel/paging.h>
#include <kernel/task.h>
#include <kernel/tss.h>
#include <kernel/device.h>
#include <kernel/vfs.h>

#include "apps/init.h"

#define init_kernel_stack(ADDR) \
	asm ( \
		"mov %0, %%esp;" \
		"mov %0, %%ebp;" \
		: \
		: "i"(ADDR) \
	);
	
#define SCHEDULE() asm("jmp context_switch;");

#define HALT() { cli(); for(;;); }

int kernel_main() {
	init_kernel_stack(KERN_HEAP_OFFSET);

	init_device_drivers();
	device_open(TTY, 0);

	k_printf("entered into main kernel..!\n");
	k_printf("and initialized device drivers.\n");

	init_gdt();
	k_printf("initialized protected mode.\n");

	check_physical_mem_size();
	k_printf("physical memory size : %d megabytes.\n", physical_mem_size>>20);

	init_paging();
	k_printf("initialized paging.\n");

	init_vfs();
	k_printf("initialized file system (fat16).\n");

	init_idt();
	k_printf("initialized interrupt.\n");

	init_tss();

	struct fat16_file_kt f;
	f = fs_open_fat16("/apps/snake.bin");
	create_task(f.buffer, f.information.file_size); 
	dump_task_queue();
	current_task = pop_task_queue();
	create_task(init1, 256);
	create_task(init2, 256);
	create_task(init3, 256);
	create_task(init4, 256);
	create_task(init5, 256);
	dump_task_queue();

	/*
	do {
		struct task_struct_kt *cur = task_head;
		do {
			k_printf("#%d - stack %x, start %x\n", cur->tid, cur->sp_page, cur->start_entry);
			cur = cur->next;
		} while (cur != task_head);
	} while(0);
	*/

	//for(;;);  // user page가 제대로 동작할때까지 막아둠.

	cli();
	SCHEDULE();
	
	for(;;);
	
	device_close(TTY, 0);

	return 0;
}
