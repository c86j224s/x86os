#include <kernel/type.h>

#ifndef __UTIL_H__
#define __UTIL_H__

int memset(void *, byte, int);
int memcpy(void *, void *, int);
int strlen(char *);

int isalpha(char);
int islower(char);
int isupper(char);
char tolower(char);
char toupper(char);

#endif

