#include <api.h>

int apiPutChar(int c) {
	return syscall1(API_PUT_CHAR, c);
}

int apiPuts(char *s) {
	return syscall1(API_PUTS, s);
}

int apiInitKeyboard(void) {
	return syscall0(API_INIT_KEYBOARD);
}

int apiReleaseKeyboard(void) {
	return syscall0(API_RELEASE_KEYBOARD);
}

int apiHasKey(void) {
	return syscall0(API_HAS_KEY);
}

int apiGetKey(void) {
	return syscall0(API_GET_KEY);
}

int apiGetCurX() {
	return syscall0(API_GET_CUR_X);
}

int apiGetCurY() {
	return syscall0(API_GET_CUR_Y);
}

int apiMoveXY(int x, int y) {
	return syscall2(API_MOVE_XY, x, y);
}

int apiDrawChar(int c) {
	return syscall1(API_DRAW_CHAR, c);
}
