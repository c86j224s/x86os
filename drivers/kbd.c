#include <kernel/idt.h>
#include <kernel/device.h>
#include <kernel/drivers/kbd.h>
#include <kernel/task.h>
#include <kernel/kio.h>
#include <kernel/util.h>

struct task_struct_kt *console = 0;

void init_device_kbd() {
	if (devices_count > KBD && devices[KBD].allocated)
		return;

	if (devices_count <= KBD)
		devices_count = KBD+1;
		
	devices[KBD].allocated = 1;
	k_sprintf(devices[KBD].device_name, "kbd");
	devices[KBD].operations = (struct device_operations) {
		.open = device_kbd_open,
		.close = device_kbd_close,
		.read = device_kbd_read,
		.write = device_kbd_write,
		.ioctl = device_kbd_ioctl,
		.lseek = device_kbd_lseek
	};
	devices[KBD].use = 0;

	// 추가적인 키보드 드라이버 설정
	// 뭐가 있을까 ㅡㅡ;;
	console = 0;

	// 키보드 인터럽트 설정
	iv_keyboard = device_kbd_interrupt;
}

void close_device_kbd() {
	devices[KBD].allocated = 0;
}

int device_kbd_open(int minor) {
	devices[KBD].use++;
	// 만약 virtual console이 할당된 태스크가 없으면 할당해줌.
	if (console == 0) {
		console = current_task;
	} else
		return -1;
	return minor;
}

int device_kbd_close(int minor) {
	devices[KBD].use--;
	// virtual console을 원래 태스크로 돌려줌.
	if (console->parent)
		console = console->parent;
	else
		console = 0;
	return 0;
}

int device_kbd_read(int minor, void *target, int length) {
	int cnt = 0;
	char *trg = (char *)target;
	while(cnt < length) {	
		// 키보드 인터럽트가 일어나서 키보드 버퍼에 
		// 키가 쌓일 때까지 기다린다. (blocking..)
		//while (console->kbd_front == console->kbd_rear);
		// 처럼 쓰려고 했으나... 블로킹 안하는게 좋겠음. -_-;;;
		if (console->kbd_front == console->kbd_rear) break;
		
		*trg++ = console->kbd_circular[console->kbd_front];
		console->kbd_front = (console->kbd_front + 1) % KBD_CIRCULAR_SIZE;

		cnt++;
	}
	return cnt;
}

int device_kbd_write(int minor, void *source, unsigned int length) {
	return -1;
}

int device_kbd_ioctl(int minor, unsigned int command, void *value) {
	switch(command) {
		default:
			return -1;
	}
	return 0;
}

int device_kbd_lseek(int minor, int offset, int start) {
	if (start != SEEK_CUR || offset <= 0) {
		return console == current_task && console->kbd_front != console->kbd_rear;
	}

	int cnt = 0;
	while (cnt < offset) {
		if (console->kbd_front == console->kbd_rear)
			break;
		console->kbd_front++;
		cnt++;
	}
	
	return cnt;
}

void device_kbd_interrupt(void) {
	unsigned char c = inb(0x60);
	if (console) {
		console->kbd_circular[console->kbd_rear] = c;
		console->kbd_rear = (console->kbd_rear + 1) % KBD_CIRCULAR_SIZE;
	}
}
