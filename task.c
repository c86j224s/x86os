#include <kernel/idt.h>
#include <kernel/task.h>
#include <kernel/paging.h>
#include <kernel/kio.h>
#include <kernel/tss.h>
#include <kernel/util.h>

dword newtask_id = 1;

struct task_struct_kt *task_head = 0;
struct task_struct_kt *current_task = 0;

void* create_task(void *start_entry, dword size) {
	struct task_struct_kt *store_task, *new_task;

	cli();

	// store current task 
	store_task = current_task;

	// allocate new page for task struct.
	current_task = alloc_page(0);
	if (current_task == 0) {
		current_task = store_task;
		sti();
		return 0;
	}

	// allocate new page for task page directory. and fill it.
	current_task->page_dir = (dword*)alloc_page(0);
	if (current_task->page_dir == 0) {
		free_page(current_task);
		current_task = store_task;
		sti();
		return 0;
	}
	memset(current_task->page_dir, 0, 4096);

	// if current task is running, copy main page directory to task page directory.
	//if (store_task != 0) {
	//	memcpy(current_task->page_dir+256, page_dir_table+256, sizeof(dword)*768);
	//}

	/* copy new task page directory to main page directory[user]. */
	memcpy(page_dir_table+256, current_task->page_dir+256, sizeof(dword)*768);
	
	// allocate new page on user space for stack.
	current_task->sp_page = alloc_page(3);
	if (current_task->sp_page == 0) {
		free_page(current_task->page_dir);
		free_page(current_task);
		if (current_task != 0)
			memcpy(page_dir_table+256, store_task->page_dir+256, sizeof(dword)*768);
		current_task = store_task;
		sti();
		return 0;
	}

	// allocate some pages on user space for program code.
	current_task->start_entry = alloc_multiple_pages(3, 
		size/4096 + (size%4096)?1:0);
	if (current_task->start_entry == 0) {
		free_page(current_task->sp_page);
		free_page(current_task->page_dir);
		free_page(current_task);
		if (store_task != 0)
			memcpy(page_dir_table+256, store_task->page_dir+256, sizeof(dword)*768);
		current_task = store_task;
		sti();
		return 0;
	}

	// copy program code to task start entry;
	memcpy(current_task->start_entry, start_entry, size);
	
	// 초기 컨텍스트를 스택에 집어넣는다.
	create_context(current_task);
	
	current_task->magic_bit = 0xEA;

	// set task id.
	current_task->tid = newtask_id++;

	// init keyboard circular queue buffer
	current_task->kbd_front = current_task ->kbd_rear = 0;

	// init descriptors
	current_task->desc_count = 0;

	// indicates who parent task is.
	current_task->parent = (store_task)? store_task:0;

	// push new task struct to task queue..
	push_task_queue(current_task);
	
	// copy main page directory[user] to new task page directory.
	memcpy(current_task->page_dir+256, page_dir_table+256, sizeof(dword)*768);
	
	// restore current task's page directory.
	if (store_task != 0)
		memcpy(page_dir_table+256, store_task->page_dir+256, sizeof(dword)*768);

	// restore current task
	new_task = current_task;
	current_task = store_task;

	sti();

	return new_task;
}

void create_context(struct task_struct_kt *task) {
	dword esp_temp;
	dword context = task->context+17;
	//dword task_esp = (byte*)task->sp_page+(4096-17*4); 
	dword task_esp = (byte*)task->sp_page+4096; 
	dword task_ebp = (byte*)task->sp_page+4096;
	asm volatile (
		"mov %%esp, %0;"
		"mov %2, %%esp;"
		"push %5;"			// ss
		"push %3;"			// esp
		"push $0x200;"		// eflags
		"push %4;"			// cs
		"push %6;"			// eip
		"push %5;"			// gs
		"push %5;"			// fs
		"push %5;"			// es
		"push %5;"			// ds
		"push $0;"			// eax
		"push $0;"			// ecx
		"push $0;"			// edx
		"push $0;"			// ebx
		"push $0;"			// (none)
		"push %6;"			// ebp
		"push $0;"			// esi
		"push $0;"			// edi
		"mov %1, %%esp;"
		: "=m"(esp_temp)
		: "m"(esp_temp),
		  "m"(context),
		  "m"(task_esp),
		  "i"(GDT_USER_CS),
		  "i"(GDT_USER_DS),
		  "m"(task->start_entry),
		  "m"(task_ebp)
	);
}

dword *schedule__esp;
dword *schedule__context;
void schedule__wrapper(void) {
	asm(
		".globl schedule;"
		"schedule:;"
		"cli;"
	);
	
	// 인터럽트 문맥을 current task에 저장함.
	// (cpu context)
	asm volatile (
		"mov %%esp, %%eax;"
		"mov %%eax, %0;"
		: "=m"(schedule__esp)
	); 
	memcpy(current_task->context, schedule__esp, 4*17);
	memcpy(current_task->page_dir+256, page_dir_table+256, sizeof(dword)*768);
	
	// 다음 태스크를 읽음.
	push_task_queue(current_task);
	current_task = pop_task_queue();
		
	asm(
		".globl context_switch;"
		"context_switch:;"
	);
	schedule__context = current_task->context;
	memcpy(page_dir_table+256, current_task->page_dir+256, sizeof(dword)*768);
	//k_printf("start_%d!", current_task->tid);
	asm volatile (
		"add $17*4, %%esp;"
		"mov %%esp, %0;"
		: "=m"(tss.esp0)
	); 
	asm volatile (
		"mov %0, %%esp;"
		"popal;"
		"pop %%ds;"
		"pop %%es;"
		"pop %%fs;"
		"pop %%gs;"
		"sti;"
		"iret;"
		:
		: "m"(schedule__context)
	);
}

void push_task_queue(struct task_struct_kt* task) {
	if (task_head == 0) {
		task_head = task;
		task->prev = task->next = task;
	} else {
		task->next = task_head;
		task->prev = task_head->prev;
		task_head->prev->next = task;
		task_head->prev = task;
	}
}

struct task_struct_kt* pop_task_queue() {
	struct task_struct_kt *temp = 0;
	if (task_head->next == task_head) {
		temp = task_head;
		task_head = 0;
	} else 	if (task_head) {
		temp = task_head;
		task_head = task_head->next;
		task_head->prev = task_head->prev->prev;
		task_head->prev->next = task_head;
	}
	return temp;
}

void dump_task_queue() {
	struct task_struct_kt *temp = task_head;
	if (temp == 0) {
		k_printf("~ task queue is empty ~\n");
		return;
	} else {
		k_printf("~ task queue contains : ");
		do {
			k_printf("#%u ", temp->tid);
			temp = temp->next;
		} while (temp != task_head);
		k_printf("~\n");
	}
}
