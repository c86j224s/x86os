#include <kernel/timer.h>
#include <kernel/pio.h>

void set_timer_interval() {
	outb(0x43, 0x34);
	outb(0x40, (TIMER_INTERVAL) & 0xff);
	outb(0x40, (TIMER_INTERVAL >> 8) & 0xff);
}
