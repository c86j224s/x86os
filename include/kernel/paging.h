#ifndef __PAGING_H__
#define __PAGING_H__

#include <kernel/type.h>
#include <kernel/addr.h>
#include <kernel/gdt.h>
#include <kernel/mem.h>

#define PGD_KERNEL(ADDR) ((dword)((ADDR) | 0x3))	// 0x1? 0x3?
#define PGD_USER(ADDR) ((dword)((ADDR) | 0x7))

#define PGT_KERNEL(ADDR) ((dword)((ADDR) | 0x3))	// 0x1? 0x3?
#define PGT_USER(ADDR) ((dword)(ADDR) | 0x7)

#define PGD_EMPTY ((dword)(0x0))
#define PGT_EMPTY ((dword)(0x0))

extern dword *page_dir_table;
extern dword *page_entry_table;

void init_paging(void);

void* alloc_page(int);
void* alloc_multiple_pages(int, int);
void free_page(void *);
void free_multiple_pages(void *, int);

#endif
