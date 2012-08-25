#include <lib.h>

unsigned int (*seed_func)(void) = 0;

void seed_rand(unsigned int (*cb)(void)) {
	seed_func = cb;
}

unsigned int random() {
	return seed_func();
}
