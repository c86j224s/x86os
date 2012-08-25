#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#define MAX_SYSCALL 256
#define SYSCALL_COUNT 6

extern void* syscall_table[];

int syscallPutChar(int);
int syscallPuts(char *);
int syscallInitKbd(void);
void syscallReleaseKbd(void);
int syscallHasKey(void);
int syscallGetKey(void);
int syscallGetCurX(void);
int syscallGetCurY(void);
void syscallMoveXY(int, int);
void syscallDrawChar(int);

/*
int syscallDevOpen(int major, int minor);
int syscallDevClose(int des);
int syscallDevRead(int des, void *target, int length);
int syscallDevWrite(int des, void *source, int length);
int syscallDevIoctl(int des, unsigned int command, void *value);
int syscallDevLseek(int des, int offset, int start);
*/

#endif
