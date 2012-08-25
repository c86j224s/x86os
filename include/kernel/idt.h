#include <kernel/type.h>
#include <kernel/gdt.h>

#ifndef __IDT_H__
#define __IDT_H__

/*********** related to PIC... ***********/
#define PIC_MASTER			0x20
#define PIC_SLAVE			0xA0
#define PIC_MASTER_COMMAND	PIC_MASTER
#define PIC_SLAVE_COMMAND	PIC_SLAVE
#define PIC_MASTER_DATA		(PIC_MASTER+1)
#define PIC_SLAVE_DATA		(PIC_SLAVE+1)

/* LTIM : edge trigger = 0, level trigger = 1 
   SNGL : master/slave = 0, master only = 1 
   IC4  : if need icw4, then 1 */
#define PIC_ICW1(LTIM,SNGL,IC4) \
	((byte)( \
		0x10 | \
		(((LTIM)&1)<<3) | \
		(((SNGL)&1)<<1) | \
		((IC4)&1) \
	))

#define PIC_ICW2(OFF) ((byte)((OFF)&0xff))

#define PIC_ICW3_MASTER(S) ((byte)((1<<(S))))
#define PIC_ICW3_SLAVE(ID) ((byte)((ID)&0x7))

/* SFNM : special fully nested mode
   AEOI : automatic eoi = 1, normal eoi = 0
   UPM  : if 8086 mode, then 1 */
#define PIC_ICW4(SFNM,AEOI,UPM) \
	((byte)( \
		(((SFNM)&1)<<4) | \
		(((AEOI)&1)<<1) | \
		((UPM)&1) \
	))

#define PIC_MASK(MASK) ((byte)(~(MASK)))
#define PIC_MASK_TIMER		0x1		/* 8253/8254 programmable interval timer */
#define PIC_MASK_KEYBOARD	0x2		/* 8042 keyboard controller */
#define PIC_MASK_SLAVE_LINE	0x4		/* 8259 interrupt line */
#define PIC_MASK_COM2_COM4	0x8		/* 8250 uart serial port com2, com4 */
#define PIC_MASK_COM1_COM3	0x10	/* 8250 uart serial port com1, com3 */
#define PIC_MASK_LPT2		0x20	/* 8255 parallel port lpt2 */
#define PIC_MASK_FLOPPY		0x40	/* 82072a floppy disk controller */
#define PIC_MASK_LPT1		0x80	/* 8255 parallel port lpt1 */

#define PIC_MASK_RTC				0x1		/* real time clock */
#define PIC_MASK_IRQ9_NOT_USED	0x2		/* no common assignment */
#define PIC_MASK_IRQ10_NOT_USED	0x4		/* no common assignment */
#define PIC_MASK_IRQ11_NOT_USED	0x8		/* no common assignment */
#define PIC_MASK_MOUSE			0x10	/* 8042 ps2 mouse controller */
#define PIC_MASK_COPROCESSOR		0x20	/* math coprocessor */
#define PIC_MASK_HARD_DISK1		0x40	/* hard disk controller 1 */
#define PIC_MASK_HARD_DISK2		0x80	/* hard disk controller 2 */

/********** related to IDT and IDTR ***********/
#define IDT_SIZE		256*8

struct idtr_kt {
	word size;
	dword position;
} __attribute__((__packed__));

#define IDTR(SIZE,POSITION) \
	((struct idtr_kt) { \
		.size = (SIZE) & 0xffff, \
		.position = (POSITION) \
	})

struct interrupt_descriptor_kt {
	word offset_15_0;
	word cs;
	byte unused;
	byte gate_type : 4;
	byte s : 1;
	byte dpl : 2;
	byte p : 1;
	word offset_31_16;
} __attribute__((__packed__));

#define INT_DESC(BASE,DPL,TRAP) \
	((struct interrupt_descriptor_kt) { \
		.offset_15_0 = (BASE) & 0xffff, \
		.cs = GDT_KERN_CS & 0xffff, \
		.unused = 0, \
		.gate_type = (0xe | ((TRAP) & 0x1)) & 0xf, \
		.s = 0, \
		.dpl = (DPL) & 0x3, \
		.p = 1, \
		.offset_31_16 = ((BASE)>>16) & 0xffff \
	})

#define IDT_TRAP_GATE					0x0
#define IDT_DE							(IDT_TRAP_GATE)
#define IDT_DB							(IDT_TRAP_GATE+1)
#define IDT_NMI_INTERRUPT				(IDT_TRAP_GATE+2)
#define IDT_BP							(IDT_TRAP_GATE+3)
#define IDT_OF							(IDT_TRAP_GATE+4)
#define IDT_BR							(IDT_TRAP_GATE+5)
#define IDT_UD							(IDT_TRAP_GATE+6)
#define IDT_NM							(IDT_TRAP_GATE+7)
#define IDT_DF							(IDT_TRAP_GATE+8)
#define IDT_COPROCESSOR_SEGMENT_OVERRUN	(IDT_TRAP_GATE+9)
#define IDT_TS							(IDT_TRAP_GATE+10)
#define IDT_NP							(IDT_TRAP_GATE+11)
#define IDT_SS							(IDT_TRAP_GATE+12)
#define IDT_GP							(IDT_TRAP_GATE+13)
#define IDT_PF							(IDT_TRAP_GATE+14)
#define IDT_MF							(IDT_TRAP_GATE+16)
#define IDT_AC							(IDT_TRAP_GATE+17)
#define IDT_MC							(IDT_TRAP_GATE+18)
#define IDT_XM							(IDT_TRAP_GATE+19)

#define IDT_INT_GATE		0x20
#define IDT_TIMER			(IDT_INT_GATE)
#define IDT_KEYBOARD 		(IDT_INT_GATE+1)

#define IDT_SINT_GATE		0x80
#define IDT_SINT0			(IDT_SINT_GATE)

/*********** functions ***********/
void init_idt(void);
void cli(void);
void sti(void);
void master_eoi(void);
void slave_eoi(void);
void idt_io_wait(void);

/*********** interrupt service routines ***********/
void isr_ignore(void);		
void isr_timer(void);		
void isr_keyboard(void);	
void isr_sint0(void);

/*********** exception trap routines ***********/
void exception_de(void);
void exception_db(void);
void exception_nmi_interrupt(void);
void exception_bp(void);
void exception_of(void);
void exception_br(void);
void exception_ud(void);
void exception_nm(void);
void exception_df(void);
void exception_coprocessor_segment_overrun(void);
void exception_ts(void);
void exception_np(void);
void exception_ss(void);
void exception_gp(void);
void exception_pf(void);
void exception_mf(void);
void exception_ac(void);
void exception_mc(void);
void exception_xm(void);

/*********** interrupt vector ***********/
// interrupt service routines call these vector functions..
extern void (*iv_keyboard)(void);

/*********** storing and restoring context for interrupt ***********/
#define STORE_CONTEXT() asm("push %gs; push %fs; push %es; push %ds; pushal;");
#define RESTORE_CONTEXT() asm("popal; pop %ds; pop %es; pop %fs; pop %gs; iret;");


#endif
