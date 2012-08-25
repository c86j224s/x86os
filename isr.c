#include <kernel/idt.h>
#include <kernel/kio.h>
#include <kernel/task.h>
#include <kernel/syscall.h>

#include "apps/init.h"

void int_schedule(void);

void isr_ignore__wrapper(void) {
	asm(
		".globl isr_ignore;"
		"isr_ignore:;"
	);
	STORE_CONTEXT();

	master_eoi();
	k_printf("I");
	
	RESTORE_CONTEXT();
}

/* test code begin */
int isr_timer__x = 0, isr_timer__y = 0;
char isr_timer__s[5] = "|/-\\";
char isr_timer__c = '|', isr_timer__cn = 0;
#define PROCESS_TICK 0x40
unsigned int isr_timer__counter = 0;
/* test code end */
void isr_timer__wrapper(void) {
	asm(
		".globl isr_timer;"
		"isr_timer:;"
	);
	STORE_CONTEXT();
	
	master_eoi();
	k_getxy(&isr_timer__x, &isr_timer__y);
	k_movexy(78, 0);
	k_putchar(isr_timer__c);
	//k_drawchar(isr_timer__c);
	isr_timer__cn = (isr_timer__cn+1<4)? (isr_timer__cn+1):0;
	isr_timer__c = isr_timer__s[isr_timer__cn];
	k_movexy(isr_timer__x, isr_timer__y);
	isr_timer__counter++;
	
	asm("jmp int_schedule;");
}

void isr_keyboard__wrapper(void) {
	asm(
		".globl isr_keyboard;"
		"isr_keyboard:;"
	);
	STORE_CONTEXT();
	
	master_eoi();
	if (iv_keyboard)
		iv_keyboard();

	RESTORE_CONTEXT();
}

int isr_sint0__eax;
int* isr_sint0__ebx;
void isr_sint0__wrapper(void) {
	asm(
		".globl isr_sint0;"
		"isr_sint0:;"
	);
	STORE_CONTEXT();
	
	asm(
		"mov %%eax, %0;"
		"mov %%ebx, %1;"
		: "=m"(isr_sint0__eax),
		  "=m"(isr_sint0__ebx)
	);
	master_eoi();
	if (isr_sint0__eax < MAX_SYSCALL) {
		*(isr_sint0__ebx+3) = 
			((int (*)(int,int,int))(syscall_table[isr_sint0__eax]))(
				*(isr_sint0__ebx),
				*(isr_sint0__ebx+1),
				*(isr_sint0__ebx+2));
	}
	
	asm("jmp int_schedule;");
	//RESTORE_CONTEXT();
}

dword int_schedule__cs;
dword int_schedule__esp_cs;
void int_schedule__wrapper(void) {
	asm(
		".globl int_schedule;"
		"int_schedule:;"
		"mov 52(%%esp), %%eax;"
		"mov %%eax, %0;"
		: "=m"(int_schedule__esp_cs)
	);
	if (int_schedule__esp_cs & 0x03 == 3 && isr_timer__counter > PROCESS_TICK) {
		isr_timer__counter = 0;
		asm("jmp schedule;");
	}
	RESTORE_CONTEXT();
}
