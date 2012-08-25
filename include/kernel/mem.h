#ifndef __MEM_H__
#define __MEM_H__

#include <kernel/type.h>

/* physical memory management */
extern byte physical_mem_table[];
extern dword physical_mem_size;

void check_physical_mem_size(void);

dword physical_mem_alloc(void);
void physical_mem_free(dword);

dword physical_mem_remaining_count(dword);

/* virtual memory management (memory pool) */

#endif
