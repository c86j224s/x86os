#include <kernel/type.h>
#include <kernel/idt.h>
#include <kernel/kio.h>

dword kdump_cs, kdump_ds, kdump_es, kdump_fs, kdump_gs, kdump_ss;
dword kdump_eax, kdump_ebx, kdump_ecx, kdump_edx;
dword kdump_ebp, kdump_esp, kdump_esi, kdump_edi;
dword kdump_eip;
#define KERNEL_DUMP() \
	do { \
		asm volatile ("mov %%eax,%0;":"=m"(kdump_eax)); \
		asm volatile ("mov %%ebx,%0;":"=m"(kdump_ebx)); \
		asm volatile ("mov %%ecx,%0;":"=m"(kdump_ecx)); \
		asm volatile ("mov %%edx,%0;":"=m"(kdump_edx)); \
		asm volatile ("mov %%cs,%%eax;mov %%eax,%0;":"=m"(kdump_cs)); \
		asm volatile ("mov %%ds,%%eax;mov %%eax,%0;":"=m"(kdump_ds)); \
		asm volatile ("mov %%es,%%eax;mov %%eax,%0;":"=m"(kdump_es)); \
		asm volatile ("mov %%fs,%%eax;mov %%eax,%0;":"=m"(kdump_fs)); \
		asm volatile ("mov %%gs,%%eax;mov %%eax,%0;":"=m"(kdump_gs)); \
		asm volatile ("mov %%ss,%%eax;mov %%eax,%0;":"=m"(kdump_ss)); \
		asm volatile ("mov %%ebp,%%eax;mov %%eax,%0;":"=m"(kdump_ebp)); \
		asm volatile ("mov %%esp,%%eax;mov %%eax,%0;":"=m"(kdump_esp)); \
		asm volatile ("mov %%esi,%%eax;mov %%eax,%0;":"=m"(kdump_esi)); \
		asm volatile ("mov %%edi,%%eax;mov %%eax,%0;":"=m"(kdump_edi)); \
		k_printf("CS[%x], DS[%x], ES[%x], FS[%x], GS[%x]\n", \
			kdump_cs, kdump_ds, kdump_es, kdump_fs, kdump_gs); \
		k_printf("EAX[%x], EBX[%x], ECX[%x], EDX[%x]\n", \
			kdump_eax, kdump_ebx, kdump_ecx, kdump_edx); \
		k_printf("EBP[%x], ESP[%x], ESI[%x], EDI[%x]\n", \
			kdump_ebp, kdump_esp, kdump_esi, kdump_edi); \
	} while(0);

dword exception__esp_v;
dword *exception__esp;
#define DUMP_EXCEPTION() \
	do{ \
		cli(); \
		KERNEL_DUMP(); \
		for(;;); \
	}while(0);

void exception_de__wrapper(void) {
	asm(".globl exception_de; exception_de:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\n#Divde error\n");
	DUMP_EXCEPTION();
}
void exception_db__wrapper(void) {
	asm(".globl exception_db; exception_db:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\n#Debug\n");
	DUMP_EXCEPTION();
}
void exception_nmi_interrupt__wrapper(void) {
	asm(".globl exception_nmi_interrupt; exception_nmi_interrupt:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\n#NMI interrupt\n");
	DUMP_EXCEPTION();
}
void exception_bp__wrapper(void) {
	asm(".globl exception_bp; exception_bp:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\n#Breakpoint\n");
	DUMP_EXCEPTION();
}
void exception_of__wrapper(void) {
	asm(".globl exception_of; exception_of:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\nOverflow\n");
	DUMP_EXCEPTION();
}
void exception_br__wrapper(void) {
	asm(".globl exception_br; exception_br:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\n#Bound range exceeded\n");
	DUMP_EXCEPTION();
}
void exception_ud__wrapper(void) {
	asm(".globl exception_ud; exception_ud:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\n#Invalid opcode\n");
	DUMP_EXCEPTION();
}
void exception_nm__wrapper(void) {
	asm(".globl exception_nm; exception_nm:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\n#Device not available\n");
	DUMP_EXCEPTION();
}
void exception_df__wrapper(void) {
	asm(".globl exception_df; exception_df:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\n#Double fault\n");
	DUMP_EXCEPTION();
}
void exception_coprocessor_segment_overrun__wrapper(void) {
	asm(".globl exception_coprocessor_segment_overrun; exception_coprocessor_segment_overrun:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\n#Coprocessor segment overrun\n");
	DUMP_EXCEPTION();
}
void exception_ts__wrapper(void) {
	asm(".globl exception_ts; exception_ts:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\n#Invalid TSS\n");
	DUMP_EXCEPTION();
}
void exception_np__wrapper(void) {
	asm(".globl exception_np; exception_np:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\n#Segment not present\n");
	DUMP_EXCEPTION();
}
void exception_ss__wrapper(void) {
	asm(".globl exception_ss; exception_ss:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\n#Stack fault\n");
	DUMP_EXCEPTION();
}
void exception_gp__wrapper(void) {
	asm(".globl exception_gp; exception_gp:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\n#General protection\n");
	DUMP_EXCEPTION();
}
void exception_pf__wrapper(void) {
	asm(".globl exception_pf; exception_pf:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\n#Page fault\n");
	DUMP_EXCEPTION();
}
void exception_mf__wrapper(void) {
	asm(".globl exception_mf; exception_mf:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\n#x87 FPU floating point error\n");
	DUMP_EXCEPTION();
}
void exception_ac__wrapper(void) {
	asm(".globl exception_ac; exception_ac:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\n#Alignment check\n");
	DUMP_EXCEPTION();
}
void exception_mc__wrapper(void) {
	asm(".globl exception_mc; exception_mc:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\n#Machine check\n");
	DUMP_EXCEPTION();
}
void exception_xm__wrapper(void) {
	asm(".globl exception_xm; exception_xm:");
	STORE_CONTEXT();
	master_eoi();
	k_printf("\n#SIMD floating point error\n");
	DUMP_EXCEPTION();
}
