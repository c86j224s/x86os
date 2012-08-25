#include <kernel/tss.h>
#include <kernel/task.h>

struct tss_kt tss = {0,};

void init_tss() {
	tss.ss0 = GDT_KERN_DS;
	tss.ss = GDT_USER_DS;
	
	asm(
		"mov %0, %%eax;"
		"ltr %%ax;"
		:
		: "i"(GDT_TSS)
	);
}
