#include <api.h>
#include <scancode.h>
#include <io.h>
#include <lib.h>

/********** 구조체, 열거타입, 유니온 선언 **********/
struct body { char x, y; } __attribute__((__packed__));
enum { SNAKE_UP, SNAKE_RIGHT, SNAKE_DOWN, SNAKE_LEFT };

/********** 글로벌 변수 선언 **********/
extern struct body snake[];
extern int snake_length;
extern int snake_direction;

extern struct body apple;

extern unsigned int d1, d2;

/********** 함수 선언 **********/
void opening(void);
void initSnake(void);
void handleKeyInput(void);
int crawlSnake(void);
void gameOver(void);

unsigned int next(void);

/********** 매크로 선언 **********/
#define DELAY(D) for(d1=0;d1<(D);d1++,d2=~d2);

/********** 메인 함수 **********/
void crt_main() {
	apiPutChar("#");
	/*
	apiInitKeyboard();

	opening();

	for(;;) {
		initSnake();
		for(;;) {
			handleKeyInput();
			if (crawlSnake()) break;
		}
		gameOver();
	}
	apiReleaseKeyboard();
	*/

	for(;;);
}

/********** 글로벌 변수 정의 **********/
struct body snake[100];
int snake_length;
int snake_direction;
struct body apple;

unsigned int d1, d2;

/********** 함수 정의 **********/
void opening() {
	int i, j;
	//unsigned int d1, d2;

	clear();
	apiMoveXY(30, 9);
	apiPuts("Mini Snake Byte Game");
	apiMoveXY(35, 10);
	apiPuts("version 0.1");

	//DELAY(0xfffff);
	//for(d1 = 0; d1 < 0xfffff; d1++) d2 = (d2+1) % 100;
	//for(d1 = 0; d1 < 0xffffff; d1++) d2 = (d2+1) % 100;
	//for(d1 = 0; d1 < 0x1fffff; d1++) d2 = (d2+1) % 100;
	DELAY(0x3fffff);
}

void initSnake() {
	int i, j;

	snake[0]  = (struct body){ .x = 1, .y = 1 };
	snake[1] = snake[2] = snake[3] = (struct body){ .x = 0, .y = 1 };
	snake_length = 4;
	snake_direction = SNAKE_RIGHT;

	clear();
	for(i = 0; i < 70; i++) {
		apiMoveXY(i, 0); apiDrawChar('#');
		apiMoveXY(i, 20); apiDrawChar('#');
	}
	for(i = 0; i < 20; i++) {
		apiMoveXY(0, i); apiDrawChar('#');
		apiMoveXY(70, i); apiDrawChar('#');
	}

	seed_rand(next);

	apple.x = random() % 69 + 1;
	apple.y = random() % 19 + 1;
}

void handleKeyInput() {
	int c;
	while (apiHasKey()) {
		c = apiGetKey();
		switch(c) {
			case SCANCODE_UP:
				if (snake_direction != SNAKE_DOWN)
					snake_direction = SNAKE_UP;
				break;
			case SCANCODE_RIGHT:
				if (snake_direction != SNAKE_LEFT)
					snake_direction = SNAKE_RIGHT;
				break;
			case SCANCODE_DOWN:
				if (snake_direction != SNAKE_UP)
					snake_direction = SNAKE_DOWN; 
					break;
			case SCANCODE_LEFT:
				if (snake_direction != SNAKE_RIGHT)
					snake_direction = SNAKE_LEFT; 
				break;
		}
	}
}

int crawlSnake() {
	int i, j;
	//unsigned int d1, d2;

	for(i = 1; i < snake_length; i++) {
		if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
			return -1;
	}

	if (snake[0].x == apple.x && snake[0].y == apple.y) {
		if (snake_length < 100) {
			snake[snake_length] = 
				snake[snake_length+1] = 
				snake[snake_length+2] =
					snake[snake_length-1];
			snake_length += 3;
		}
		apple.x = random() % 69 + 1;
		apple.y = random() % 19 + 1;
	}

	apiMoveXY(snake[snake_length-1].x, snake[snake_length-1].y);
	apiDrawChar(' ');

	for(i = snake_length-1; i > 0; i--)
		snake[i] = snake[i-1];
	switch(snake_direction) {
		case SNAKE_UP:		snake[i].y--; break;
		case SNAKE_RIGHT:	snake[i].x++; break;
		case SNAKE_DOWN:	snake[i].y++; break;
		case SNAKE_LEFT:	snake[i].x--; break;
	}
	if (snake[0].x < 1 || snake[0].x >= 70 || snake[0].y < 1 || snake[0].y >= 20) {
		return -1;
	}
	
	apiMoveXY(apple.x, apple.y);
	apiDrawChar('a');

	for(i = snake_length-1; i >= 0; i--) {
		apiMoveXY(snake[i].x, snake[i].y);
		apiDrawChar('@');
	}

	//DELAY(0x4fffff);
	//for(d1 = 0, d2 = 0; d1 < 0x4fffff; d1++) d2 = d1;
	//for(d1 = 0, d2 = 0; d1 < 0xffffff; d1++) d2 = d1;
	//for(d1 = 0, d2 = 0; d1 < 0x1fffff; d1++) d2 = d1;
	DELAY(0x3fffff);

	return 0;
}

void gameOver() {
	unsigned int d1, d2;

	clear();
	apiMoveXY(30, 10);
	apiPuts("GAME OVER !!!");

	//DELAY(0xfffff);
	//for(d1 = 0; d1 < 0x1fffff; d1++) d2 = ~d2;
}

unsigned int next() {
	static int s;
	s >>= 14;
	s += 3;
	return s;
}
