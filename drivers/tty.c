#include <kernel/pio.h>
#include <kernel/device.h>
#include <kernel/kio.h>
#include <kernel/drivers/tty.h>
#include <kernel/util.h>

#define TTY_BYTE_COLOR(BG,FG) ((((BG) & 0xf) << 4) | ((FG) & 0xf))
#define TTY_SCR(Y,X,B) ((unsigned char*)(0xB8000 + ((Y)*80+(X))*2 + (B)))

struct tty_conf_kt tty_setting;

void init_device_tty() {
	if (devices_count > TTY && devices[TTY].allocated)
		return;

	if (devices_count <= TTY)
		devices_count = TTY+1;
		
	devices[TTY].allocated = 1;
	k_sprintf(devices[TTY].device_name, "tty");
	devices[TTY].operations = (struct device_operations) {
		.open = device_tty_open,
		.close = device_tty_close,
		.read = device_tty_read,
		.write = device_tty_write,
		.ioctl = device_tty_ioctl,
		.lseek = device_tty_lseek
	};
	devices[TTY].use = 0;

	tty_setting.x = tty_setting.y = 0;
	tty_setting.bgcolor = 0x0;
	tty_setting.fgcolor = 0xf;
}

void close_device_tty() {
	devices[TTY].allocated = 0;
}

int device_tty_open(int minor) {
	devices[TTY].use++;
	return minor;
}

int device_tty_close(int minor) {
	devices[TTY].use--;
	return 0;
}

int device_tty_read(int minor, void *target, int length) {
	return -1;
}

int device_tty_write(int minor, void *source, unsigned int length) {
	int ret = 0;
	int i;

	while (ret < length) {
		int scroll = 0;

		switch(*(char*)source) {
			case '\n':
				while (tty_setting.x < 80) {
					*(char*)TTY_SCR(tty_setting.y,tty_setting.x,0) = *(char*)' ';
					*(char*)TTY_SCR(tty_setting.y,tty_setting.x,1) = TTY_BYTE_COLOR(tty_setting.bgcolor,tty_setting.fgcolor);
					tty_setting.x++;
				}
				tty_setting.x = 0;
				tty_setting.y++;
				break;
			case '\r':
				tty_setting.x = 0;
				break;
			case '\t':
				tty_setting.x+=tty_setting.tabswidth;
				break;
			default:
				*(char*)TTY_SCR(tty_setting.y,tty_setting.x,0) = *(char*)source++;
				*(char*)TTY_SCR(tty_setting.y,tty_setting.x,1) = TTY_BYTE_COLOR(tty_setting.bgcolor,tty_setting.fgcolor);
				tty_setting.x++;
				break;
		}
		
		if (tty_setting.x >= 80) {
			tty_setting.y += tty_setting.x / 80;
			tty_setting.x %= 80;
		}
		if (tty_setting.y >= 25) {
			int i;
			scroll = tty_setting.y - 24;
			tty_setting.y = 24;
		}

		if (scroll) {
			memcpy(TTY_SCR(0,0,0), TTY_SCR(scroll,0,0), (25-scroll)*80*2);
			for(i = tty_setting.x; i < 80; i++) {
				*(char*)TTY_SCR(24,i,0) = *(char*)' ';
				*(char*)TTY_SCR(24,i,1) = TTY_BYTE_COLOR(tty_setting.bgcolor,tty_setting.fgcolor);
			}
		}

		ret++;
	}
	return ret;
}

int device_tty_ioctl(int minor, unsigned int command, void *value) {
	struct tty_conf_kt *p = (struct tty_conf_kt *)value;
	switch(command) {
		case TTY_COMMAND_GET_CONF:
			p->x = tty_setting.x;
			p->y = tty_setting.y;
			p->bgcolor = tty_setting.bgcolor;
			p->fgcolor = tty_setting.fgcolor;
			break;
		case TTY_COMMAND_SET_CONF:
			if (p->x < 0 || p->x >= 80 || p->y < 0 || p->y >= 25)
				return -1;
			tty_setting.x = p->x;
			tty_setting.y = p->y;
			tty_setting.bgcolor = p->bgcolor;
			tty_setting.fgcolor = p->fgcolor;
			break;
		case TTY_COMMAND_RELOCATE_CURSOR:
			do {
				int offset = tty_setting.y * 80 + tty_setting.x;
				// cursor low port to vga index register
				outb(0x3d4, 0x0f);
				outb(0x3d5, (unsigned char)(offset & 0xff));
				// cursor high port to vga index register
				outb(0x3d4, 0x0e);
				outb(0x3d5, (unsigned char)((offset>>8) & 0xff));
			} while (0);
			break;
		default:
			return -1;
	}
	return 0;
}

int device_tty_lseek(int minor, int offset, int start) {
	switch(start) {
		case SEEK_SET:
			break;
		case SEEK_CUR:
			offset += tty_setting.y * 80 + tty_setting.x;
			break;
		case SEEK_END:
			offset = 80*25 + offset;
			break;
		default:
			return -1;
	}
	if (offset >= 80*25 || offset < 0)
		return -1;
	tty_setting.x = offset % 80;
	tty_setting.y = offset / 80;

	return 0;
}
