#ifndef __ADDR_H__
#define __ADDR_H__

#define RESERVED1_OFFSET 0x0
#define GDT_OFFSET 0x10000
#define IDT_OFFSET 0x11000
#define PAGE_DIR_OFFSET 0x12000
#define PAGE_TBL_OFFSET 0x100000
#define KERNEL_OFFSET 0x500000
#define KERN_HEAP_OFFSET 0x800000

#define USER_STACK_PAGE_OFFSET 0x40000000

#endif
