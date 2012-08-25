#include <kernel/util.h>

int memset(void *dst, byte c, int count) {
	int i;
	for (i = 0; i < count; i++) 
		*(byte*)(dst+i) = c;
	return i;
}

int memcpy(void *dst, void *src, int count) {
	int i;
	for (i = 0; i <count; i++)
		*(byte*)(dst+i) = *(byte*)(src+i);
	return i;
}

void* memmove (void *dst, const void *src, dword count) {
	int i;
	for (i = 0; i < count; i++)
		*(byte*)(dst+i) = *(byte*)(src+i);
	return dst;
}

int strlen(char *s) {
	int len = 0;
	while (*s++) len++;
	return len;
}

int isalpha(char c) {
	return (c >= 'a' && c < 'z') || (c >= 'A' && c <= 'Z');
}

int islower(char c) {
	return c >= 'a' && c < 'z';
}

int isupper(char c) {
	return c >= 'A' && c < 'Z';
}

char tolower(char c) {
	return isupper(c)? (c-'A'+'a'):c;
}

char toupper(char c) {
	return islower(c)? (c-'a'+'A'):c;
}
