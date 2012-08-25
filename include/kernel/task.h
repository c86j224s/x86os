#ifndef __PROCESS_H__
#define __PROCESS_H__

#include<kernel/type.h>
#include <kernel/device.h>

#define KBD_CIRCULAR_SIZE 128
#define MAX_DESCRIPTORS 64 // 최대 파일(디바이스) 디스크립터 수

struct task_struct_kt {
	/* task id */
	dword tid;
	
	/* user mode page directory offset */
	dword *page_dir;

	/* stack page */
	void *sp_page;
	
	/* task context */
	dword context[17];
	
	/* static code offset = ebp */
	void *start_entry;
	
	/* parent task */
	struct task_struct_kt *parent;

	/* keyboard circular queue buffer */
	char kbd_circular[KBD_CIRCULAR_SIZE];
	int kbd_front, kbd_rear;

	/* descriptors */
	struct device_descriptor_kt descs[MAX_DESCRIPTORS];
	int desc_count;

	/* for linked list structure */
	struct task_struct_kt *prev, *next;
	
	byte magic_bit;
} __attribute__ ((__packed__));

extern struct task_struct_kt *task_head; // 다음에 실행될 태스크
extern struct task_struct_kt *current_task;  // 현재 실행중인 태스크

/* 새로운 태스크를 생성해서 태스크큐의 맨 뒤에 추가한다. */
void *create_task(void *, dword);
/* 태스크 문맥을 스택에 저장한다. */
void create_context(struct task_struct_kt *);

/* 스케줄링을 실행하는 함수 (어셈블리 루틴) */
void schedule(void);
void context_switch(void);

/* 태스크 큐 관련 함수들 (태스크큐는 원형큐다..) */
void push_task_queue(struct task_struct_kt *);
struct task_struct_kt *pop_task_queue(void);
void dump_task_queue(void);

#endif
