#include <kernel/type.h>

#ifndef __GDT_H__
#define __GDT_H__

#define PRIV_KERN 0				// kernel privilege mode
#define PRIV_USER 3				// user privilege mode

#define TYPE_CODE 0x0a			// segment descriptor type for code
#define TYPE_DATA 0x02			// segment descriptor type for data

#define GDT_SEG_SEL(NUM,PRIV) (0x08*(NUM) + (PRIV))

#define GDT_KERN_CS GDT_SEG_SEL(1,PRIV_KERN)
#define GDT_KERN_DS GDT_SEG_SEL(2,PRIV_KERN)
#define GDT_USER_CS GDT_SEG_SEL(3,PRIV_USER)
#define GDT_USER_DS GDT_SEG_SEL(4,PRIV_USER)

struct gdtr_kt {
	word size;
	dword position;
} __attribute__((__packed__));

struct segment_descriptor_kt {
	word limit_15_0;
	word base_15_0;
	byte base_23_16;
	byte type : 4;
	byte s : 1;
	byte dpl : 2;
	byte p : 1;
	byte limit_19_16 : 4;
	byte avl : 1;
	byte reserved : 1;
	byte d : 1;
	byte g : 1;
	byte base_31_24;
} __attribute__((__packed__));

#define GDTR(SIZE,POSITION) \
	((struct gdtr_kt) { \
		.size = (SIZE) & 0xffff, \
		.position = (POSITION) \
	})

#define SEG_DESC(LIMIT,BASE,TYPE,DPL) \
	((struct segment_descriptor_kt) { \
		.limit_15_0 = (LIMIT) & 0xffff, \
		.base_15_0 = (BASE) & 0xffff, \
		.base_23_16 = ((BASE) >> 16) & 0xff, \
		.type = (TYPE) & 0x0f, \
		.s = 1, \
		.dpl = (DPL) & 0x3, \
		.p = 1, \
		.limit_19_16 = ((LIMIT) >> 16) & 0x0f, \
		.avl = 0, \
		.reserved = 0, \
		.d = 1, \
		.g = 1, \
		.base_31_24 = ((BASE) >> 24) & 0xff \
	})

void init_gdt(void);

#endif
