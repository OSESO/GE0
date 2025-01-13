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

int spritegetvalue(int n, enum sprite_attr type) {
    return getSpriteValue(n, type);
}

void spritesetvalue(int n, enum sprite_attr type, int value) {
    setSpriteValue(n, type, value);
}

int angbetweenspr(int n1, int n2) { return angleBetweenSprites(n1, n2); }

void getsprite(int n, int address) { setSpr(n, address); }

void putsprite(int n, int x, int y) {
    setSprPosition(n, x, y);
    if (getSpriteValue(n, S_LIVES) < 1) {
        setSpriteValue(n, S_LIVES, 1);
    }
}

void spritespeed(int n, int speed, int direction) {
    spriteSetDirectionAndSpeed(n, speed, direction);
}

int getspriteinxy(int x, int y) { return getSpriteInXY(x, y); }

void putpixel(int x, int y) { setPix(x, y, color); }

int getpixel(int x, int y) { return getPix(x, y); }

void line(int x, int y, int x1, int y1) { drwLine(x1, y1, x1, y1); }

void rect(int x, int y, int x1, int y1) { drwRect(x, y, x1, y1); }

void fillrect(int x, int y, int x1, int y1) { fllRect(x, y, x1, y1); }

void triangle(int x, int y, int x1, int y1, int x2, int y2) {
    drwTriangle(x, y, x1, y1, x2, y2);
}

void filltriangle(int x, int y, int x1, int y1, int x2, int y2) {
    fllTriangle(x, y, x1, y1, x2, y2);
}

void circle(int x, int y, int r) { drwCirc(x, y, r); }

void fillcircle(int x, int y, int r) { fllCirc(x, y, r); }
char putchar(char c) {
    printc(c, color, bgcolor);
    return c;
}

int puts(char *message) {
    int j = 0;
    while (*(message + j) != 0 && j <= 1000) {
        printc(*(message + j), color, bgcolor);
    }
    return 0;
}

// FIXME: Klib hack. Get rid of it.
int klib__itoa(int num, char *str, int radix);
#define itoa klib__itoa
int putn(int number) {
    char s_buffer[7];
    if (number < 32768) {
        itoa(number, s_buffer, 10);
    } else {
        itoa(number - 0x10000, s_buffer, 10);
    }
    int j = 0;
    while (s_buffer[j]) {
        printc(s_buffer[j], color, bgcolor);
        j++;
    }
    return 0;
}

int printf(char *formatString, arg - list...) {}

int getchar() {}

void loadfont(char *font, char first, char end) {}

void setfontsize(int picture_width, int picture_height, int character_width,
                 int character_height) {}

void drawstring(char *text, int x, int y) {}

void drawchar(char character, int x, int y) {}
