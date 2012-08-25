#include <kernel/idt.h>
#include <kernel/lock.h>

struct lock_kt init_lock() {
	struct lock_kt temp;
	temp.lock = 0;
	return temp;
}

int try_lock(struct lock_kt *l) {
	cli();
	int rv = l->lock;
	l->lock = 1;
	sti();
	return rv;
}

void acquire_lock(struct lock_kt *l) {
	while(try_lock(l));	
}

void release_lock(struct lock_kt *l) {
	l->lock = 0;
}
