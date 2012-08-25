int cursor = 0;

void print_chars(char *str) {
	char *scr = (unsigned char *)0xB8000, 
		*c = str;

	while (*c != 0) {
		if (*c == '\n') {
			cursor = cursor / 160 * 160 + 160;
			c++;
			continue;
		}
		*(scr + cursor++) = *c++;
		*(scr + cursor++) = 0x06;
	}
}
