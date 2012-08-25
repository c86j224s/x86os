#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <type.h>

uint32 syscall0(uint32);
uint32 syscall1(uint32, uint32);
uint32 syscall2(uint32, uint32, uint32);
uint32 syscall3(uint32, uint32, uint32, uint32);

#endif
