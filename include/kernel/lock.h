#ifndef __LOCK_H__
#define __LOCK_H__

struct lock_kt {
	// process pointer
	int lock;
} __attribute__((__packed__));

struct lock_kt init_lock();
int try_lock(struct lock_kt *);
void acquire_lock(struct lock_kt *);
void release_lock(struct lock_kt *);

#endif
