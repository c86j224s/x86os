#include <kernel/kio.h>
#include <kernel/device.h>
#include <kernel/util.h>

int k_getxy(int *x, int *y) {
	struct tty_conf_kt tc;
	device_ioctl(TTY, 0, TTY_COMMAND_GET_CONF, &tc);
	*x = tc.x;
	*y = tc.y;
	return 0;
}

int k_movexy(int x, int y) {
	struct tty_conf_kt tc;
	device_ioctl(TTY, 0, TTY_COMMAND_GET_CONF, &tc);
	tc.x = x;
	tc.y = y;
	device_ioctl(TTY, 0, TTY_COMMAND_SET_CONF, &tc);
	return 0;
}

int k_getcolor(int *bgcolor, int *fgcolor) {
	struct tty_conf_kt tc;
	device_ioctl(TTY, 0, TTY_COMMAND_GET_CONF, &tc);
	*bgcolor = tc.bgcolor;
	*fgcolor = tc.fgcolor;
	return 0;
}

int k_setcolor(int bgcolor, int fgcolor) {
	struct tty_conf_kt tc;
	device_ioctl(TTY, 0, TTY_COMMAND_GET_CONF, &tc);
	tc.bgcolor = bgcolor;
	tc.fgcolor = fgcolor;
	device_ioctl(TTY, 0, TTY_COMMAND_SET_CONF, &tc);
	return 0;
}

int k_putchar(int c) {
	int ret = device_write(TTY, 0, &c, 1);
	device_ioctl(TTY, 0, TTY_COMMAND_RELOCATE_CURSOR, 0);
	return ret;
}

int k_puts(char *s) {
	int ret = 0;
	while(*s != 0) {
		k_putchar(*s++);
		ret++;
	}
	return ret;
}

int k_drawchar(int c) {
	return device_write(TTY, 0, &c, 1);
}

int k_draws(char *s) {
	int ret = 0;
	while(*s != 0) {
		k_drawchar(*s++);
		ret++;
	}
	return ret;
}

/****** k_printf(fmt,...) 에 사용되는 함수군 ******/
char *binary(char *target, unsigned int n) {
	char temp[32], i = 0;
	
	while (n > 0) {
		temp[i++] = (n & 1)? '1':'0';
		n >>= 1;
	}
	for(i-=1; i >= 0; i--) {
		*target++ = temp[i];
	}
	return target;
}

char *hexa(char *target, unsigned int n) {
	char sym[17] = "0123456789ABCDEF";
	char temp[8];
	int i = 0;
	
	if (n == 0) {
		*target++ = '0';
		return target;
	}
	
	while (n > 0) {
		temp[i++] = sym[n & 0x0f];
		temp[i++] = sym[(n>>4) & 0x0f];
		n >>= 8;
	}
	for(i-=1; i >= 0; i--) {
		*target++ = temp[i];
	}
	return target;
}

char *unsigned_number(char *target, unsigned long long n, int size) {
	char temp[128];
	int i = 0;
	
	if (n == 0) {
		*target++ = '0';
		return target;
	}

	while (n > 0) {
		temp[i++] = (n % 10) + '0';
		n /= 10;
	}
	for(i-=1; i >= 0; i--) {
		*target++ = temp[i];
	}
	return target;
}

char *signed_number(char *target, long long n, int size) {
	char temp[128], s = 0;
	int i = 0;
	
	if (n == 0) {
		*target++ = '0';
		return target;
	}

	if (n < 0) {
		s = 1;
		n *= -1;
	}
	
	while (n > 0) {
		temp[i++] = (n % 10) + '0';
		n /= 10;
	}
	if (s)
		*target++ = '-';
	for(i-=1; i >= 0; i--) {
		*target++ = temp[i];
	}
	return target;
}

char *string(char *target, char *s) {
	while(*s) {
		*target++ = *s++;
	}
	return target;
}

int k_vsprintf(char *target, const char *fmt, va_list args) {
	char c;
	const char *old_target = target;
	while (*fmt) {
		if (*fmt == '%') {
			*fmt++;
			switch(*fmt) {
				case 'c':
					c = (char)va_arg(args, int);
					*target++ = c;
					break;
				case '%':
					*target++ = '%';
					break;
				case 's':
					target = string(target, va_arg(args, char*));
					break;
				case 'b':
					target = binary(target, va_arg(args, int));
					break;
				case 'x':
					target = hexa(target, va_arg(args, int));
					break;
				case 'u':
					target = unsigned_number(target, va_arg(args, int), 4);
					break;
				case 'd':
					target = signed_number(target, va_arg(args, int), 4);
					break;
				default:
					*target++ = '%';
					*target++ = *fmt;
			}
			fmt++;
			continue;
		}
		*target++ = *fmt++;
	}
	*target = 0;
	return (int)target - (int)old_target;
}

int k_sprintf(char *target, char *fmt, ...) {
	va_list va;
	int nl;

	va_start(va, fmt);
	nl = k_vsprintf(target, fmt, va);
	va_end(va);

	return nl;
}

int k_printf(char *fmt, ...) {
	va_list va;
	int nl;
	char text[1024];
	
	va_start(va, fmt);
	nl = k_vsprintf(text, fmt, va);
	va_end(va);

	k_puts(text);
	
	return nl;
}
