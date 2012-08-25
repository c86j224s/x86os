#ifndef __API_H__
#define __API_H__

#include <syscall.h>

#define API_PUT_CHAR 0
#define API_PUTS 1
#define API_INIT_KEYBOARD 2
#define API_RELEASE_KEYBOARD 3
#define API_HAS_KEY 4
#define API_GET_KEY 5
#define API_GET_CUR_X 6
#define API_GET_CUR_Y 7
#define API_MOVE_XY 8
#define API_DRAW_CHAR 9

int apiPutChar(int);
int apiPuts(char*);
int apiInitKeyboard(void);
void apiReleaseKeyboard(void);
int apiHasKey(void);
int apiGetKey(void);
int apiGetCurX(void);
int apiGetCurY(void);
void apiMoveXY(int, int);
int apiDrawChar(int);

#endif
