#include <syscall.h>

uint32 p[4];

uint32 syscall0(uint32 function) {
	p[0] = p[1] = p[2] = p[3] = 0;
	asm volatile ("int $0x80;":: "a"(function), "b"(p));
	return p[3];
}

uint32 syscall1(uint32 function, uint32 param1) {
	p[0] = param1; p[1] = p[2] = p[3] = 0;
	asm volatile ("int $0x80;":: "a"(function), "b"(p));
	return p[3];
}

uint32 syscall2(uint32 function, uint32 param1, uint32 param2) {
	p[0] = param1; p[1] = param2; p[2] = p[3] = 0;
	asm volatile ("int $0x80;":: "a"(function), "b"(p));
	return p[3];
}

uint32 syscall3(uint32 function, uint32 param1, uint32 param2, uint32 param3) {
	p[0] = param1; p[1] = param2; p[2] = param3; p[3] = 0;
	asm volatile ("int $0x80;":: "a"(function), "b"(p));
	return p[3];
}
