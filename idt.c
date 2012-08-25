#include <kernel/idt.h>
#include <kernel/pio.h>
#include <kernel/kio.h>
#include <kernel/timer.h>
#include <kernel/addr.h>

void (*iv_keyboard)(void) = 0;	// 엥? 얜 왜 여기..

void init_interrupt_services(void);
void init_exception_traps(void);

void init_idt () {
	struct interrupt_descriptor_kt *int_desc;
	struct idtr_kt idtr;

	cli();
	
	// initialization command (icw1)
	outb(PIC_MASTER_COMMAND, PIC_ICW1(0,0,1));
	idt_io_wait();
	outb(PIC_SLAVE_COMMAND, PIC_ICW1(0,0,1));
	idt_io_wait();

	// icw2
	outb(PIC_MASTER_DATA, PIC_ICW2(IDT_INT_GATE));
	idt_io_wait();
	outb(PIC_SLAVE_DATA, PIC_ICW2(IDT_INT_GATE+8));
	idt_io_wait();
	
	// icw3
	outb(PIC_MASTER_DATA, PIC_ICW3_MASTER(2));
	idt_io_wait();
	outb(PIC_SLAVE_DATA, PIC_ICW3_SLAVE(2));
	idt_io_wait();

	// icw4
	outb(PIC_MASTER_DATA, PIC_ICW4(0,0,1));
	idt_io_wait();
	outb(PIC_SLAVE_DATA, PIC_ICW4(0,0,1));
	idt_io_wait();
	
	// mask interrupts
	outb(PIC_MASTER_DATA, PIC_MASK(0));
	idt_io_wait();
	outb(PIC_SLAVE_DATA, PIC_MASK(0));
	idt_io_wait();

	// making ignorable interrupt descriptor table
	for ( int_desc = IDT_OFFSET
		; int_desc < IDT_OFFSET + IDT_SIZE
		; int_desc++ ) {

		*int_desc = INT_DESC((int)isr_ignore, PRIV_KERN, 0);
	}

	init_interrupt_services();	
	init_exception_traps();

	idtr = IDTR(IDT_SIZE, IDT_OFFSET);
	asm (
		"lidt %0;"
		:
		: "m"(idtr)
	);
	outb(PIC_MASTER_DATA, PIC_MASK(PIC_MASK_TIMER | PIC_MASK_KEYBOARD));
	idt_io_wait();
	outb(PIC_SLAVE_DATA, PIC_MASK(0));
	idt_io_wait();

	set_timer_interval();
	sti();
}

void init_interrupt_services() {
	struct interrupt_descriptor_kt *int_desc;

	// making timer interrupt descriptor
	int_desc = IDT_OFFSET + IDT_TIMER * 8;
	*int_desc = INT_DESC((int)isr_timer, PRIV_KERN, 0);

	// making keyboard interrupt descriptor
	int_desc = IDT_OFFSET + IDT_KEYBOARD * 8;
	*int_desc = INT_DESC((int)isr_keyboard, PRIV_KERN, 0);
	
	// making software interrupt 0 descriptor
	int_desc = IDT_OFFSET + IDT_SINT0 * 8;
	*int_desc = INT_DESC((int)isr_sint0, PRIV_USER, 1);
}

void init_exception_traps() {
	struct interrupt_descriptor_kt *int_desc;
	
	// if descriptor is for trap, INT_DESC(~~, 3, 1)
	// here
	
	int_desc = IDT_OFFSET + IDT_DE * 8;
	*int_desc = INT_DESC((int)exception_de, PRIV_USER, 1);

	int_desc = IDT_OFFSET + IDT_DB * 8;
	*int_desc = INT_DESC((int)exception_db, PRIV_USER, 1);
	
	int_desc = IDT_OFFSET + IDT_NMI_INTERRUPT * 8;
	*int_desc = INT_DESC((int)exception_nmi_interrupt, PRIV_USER, 1);
	
	int_desc = IDT_OFFSET + IDT_BP * 8;
	*int_desc = INT_DESC((int)exception_bp, PRIV_USER, 1);

	int_desc = IDT_OFFSET + IDT_OF * 8;
	*int_desc = INT_DESC((int)exception_of, PRIV_USER, 1);
	
	int_desc = IDT_OFFSET + IDT_BR * 8;
	*int_desc = INT_DESC((int)exception_br, PRIV_USER, 1);	

	int_desc = IDT_OFFSET + IDT_UD * 8;
	*int_desc = INT_DESC((int)exception_ud, PRIV_USER, 1);

	int_desc = IDT_OFFSET + IDT_NM * 8;
	*int_desc = INT_DESC((int)exception_nm, PRIV_USER, 1);

	int_desc = IDT_OFFSET + IDT_DF * 8;
	*int_desc = INT_DESC((int)exception_df, PRIV_USER, 1);
	
	int_desc = IDT_OFFSET + IDT_COPROCESSOR_SEGMENT_OVERRUN * 8;
	*int_desc = INT_DESC((int)exception_coprocessor_segment_overrun, PRIV_USER, 1);

	int_desc = IDT_OFFSET + IDT_TS * 8;
	*int_desc = INT_DESC((int)exception_ts, PRIV_USER, 1);

	int_desc = IDT_OFFSET + IDT_NP * 8;
	*int_desc = INT_DESC((int)exception_np, PRIV_USER, 1);

	int_desc = IDT_OFFSET + IDT_SS * 8;
	*int_desc = INT_DESC((int)exception_ss, PRIV_USER, 1);

	int_desc = IDT_OFFSET + IDT_GP * 8;
	*int_desc = INT_DESC((int)exception_gp, PRIV_USER, 1);

	int_desc = IDT_OFFSET + IDT_PF * 8;
	*int_desc = INT_DESC((int)exception_pf, PRIV_USER, 1);

	int_desc = IDT_OFFSET + IDT_MF * 8;
	*int_desc = INT_DESC((int)exception_mf, PRIV_USER, 1);

	int_desc = IDT_OFFSET + IDT_AC * 8;
	*int_desc = INT_DESC((int)exception_ac, PRIV_USER, 1);

	int_desc = IDT_OFFSET + IDT_MC * 8;
	*int_desc = INT_DESC((int)exception_mc, PRIV_USER, 1);

	int_desc = IDT_OFFSET + IDT_XM * 8;
	*int_desc = INT_DESC((int)exception_xm, PRIV_USER, 1);
}

void cli() {
	asm ("cli;");
}

void sti() {
	asm ("sti;");
}

void master_eoi() {
	outb(PIC_MASTER_COMMAND, 0x20);
}

void slave_eoi() {
	outb(PIC_SLAVE_COMMAND, 0x20);
}

void idt_io_wait() {
	asm (
		".byte 0xeb;"
		".byte 0x00;"
		".byte 0xeb;"
		".byte 0x00;"
	);
}
