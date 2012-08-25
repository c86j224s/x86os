#include <kernel/gdt.h>
#include <kernel/tss.h>
#include <kernel/addr.h>

void init_gdt(void) {
	struct segment_descriptor_kt *gdt = (struct segment_descriptor_kt *)GDT_OFFSET;
	struct segment_descriptor_kt temp = {0,};
	struct gdtr_kt gdtr;
	*gdt++ = temp;
	*gdt++ = SEG_DESC(0xfffff, 0, TYPE_CODE, PRIV_KERN);
	*gdt++ = SEG_DESC(0xfffff, 0, TYPE_DATA, PRIV_KERN);
	*gdt++ = SEG_DESC(0xfffff, 0, TYPE_CODE, PRIV_USER);
	*gdt++ = SEG_DESC(0xfffff, 0, TYPE_DATA, PRIV_USER);
	*gdt++ = TSS_DESC(0x68, (int)&tss);
	gdtr = GDTR((unsigned short)gdt-GDT_OFFSET, GDT_OFFSET);
	asm(
		"cli;"
		"lgdt %0;"
		"mov %%cr0, %%eax;"
		"or $0x1, %%eax;"
		"mov %%eax, %%cr0;"
		"ljmp %1, $pm;"
		"nop;"
		"nop;"
		"pm: mov %2, %%ax;"
		"mov %%ax, %%ds;"
		"mov %%ax, %%es;"
		"mov %%ax, %%fs;"
		"mov %%ax, %%gs;"
		"mov %%ax, %%ss;"
		:
		: "m"(gdtr),
		  "i"(GDT_KERN_CS),
		  "i"(GDT_KERN_DS)
	);
}
