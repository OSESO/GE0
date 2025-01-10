#include "engine.h"
#include "screen.h"

int8_t color = 1;
int8_t bgcolor = 0;
uint8_t timeForRedraw = 48;

void setimagesize(fixed size) { setImageSize(size); }

void setpalette(int n, int r5g6b5) {
    // NOTE: 数据类型有些不一样
    changePalette(n, r5g6b5);
}

void setcolor(int _color) { color = _color & 0xF; }

void setbgcolor(int _color) { bgcolor = _color & 0xF; }

void setclip(int x, int y, int width, int height) {
    setClip(x, y, width, height);
}

void gotoxy(int x, int y) {
    setCharX(x);
    setCharY(y);
}

void scroll(int dir) {
    // 这里两个执行器实现的不一样，js版中，当dir=0 or 2 时，会执行两次
    // 不知道为啥
    scrollScreen(1, dir);
}

void delayredraw() {
    for (volatile int d = 10000; d > 0; d--) {
        for (volatile int e = 1000; e > 0; e--) {
            ;
        }
    }
}
void clearscreen() { clearScr(bgcolor); }

void setframerate(int fps) {
    if (fps < 1 || fps > 40)
        return;
    timeForRedraw = 1000 / fps;
}

void putimage(char *image, int x, int y, int w, int h) {
    drawImg((unsigned char *)image, x, y, w, h);
}
void putimage1bit(char *image, int x, int y, int w, int h) {
    drawImageBit((unsigned char *)image, x, y, w, h);
}

void putimagerle(char *image, int x, int y, int w, int h) {
    drawImgRLE((unsigned char *)image, x, y, w, h);
}
