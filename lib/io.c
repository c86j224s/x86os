#include <io.h>

void clear() {
	int i;
	apiMoveXY(0, 0);
	for(i = 0; i < 25; i++)
		apiPutChar('\n');
	apiMoveXY(0, 0);
}
