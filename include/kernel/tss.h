#include <kernel/gdt.h>
#include <kernel/type.h>

#ifndef __TSS_H__
#define __TSS_H__

#define GDT_TSS GDT_SEG_SEL(5,PRIV_KERN)

#define TSS_DESC(LIMIT,BASE) \
	((struct segment_descriptor_kt) { \
		.limit_15_0 = (LIMIT) & 0xffff, \
		.base_15_0 = (BASE) & 0xffff, \
		.base_23_16 = ((BASE) >> 16) & 0xff, \
		.type = 0x09, \
		.s = 0, \
		.dpl = 0, \
		.p = 1, \
		.limit_19_16 = ((LIMIT) >> 16) & 0x0f, \
		.avl = 0, \
		.reserved = 0, \
		.d = 0, \
		.g = 0, \
		.base_31_24 = ((BASE) >> 24) & 0xff \
	})
	
struct tss_kt {
	word back_link, res0;
	dword esp0;
	word ss0, res1;
	dword esp1;
	word ss1, res2;
	dword esp2;
	word ss2, res3;
	dword cr3;
	dword eip;
	dword eflags;
	dword eax;
	dword ecx;
	dword edx;
	dword ebx;
	dword esp;
	dword ebp;
	dword esi;
	dword edi;
	word es, res4;
	word cs, res5;
	word ss, res6;
	word ds, res7;
	word fs, res8;
	word gs, res9;
	word ldt, res10;
	word t:1, res11:15, iobase;
} __attribute__((__packed__));

extern struct tss_kt tss;

/* 너는 이런 일들을 하면 된다 :
	TSS 테이블을 세팅한다. 
	최초의 태스크인 init()을 만든다. (별도의 함수가 필요한데..)
		init() 함수를 하나의 별도의 페이지에 복사한다.
		create_task로 init() 함수를 태스크로 만든다.
		스케줄링을 시작한다.
*/
void init_tss(void);

#endif
