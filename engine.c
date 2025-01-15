#include "engine.h"
#include "ge0_port_interface.h"
#include "screen.h"
#include "sound.h"
#include "stdarg.h"
#include <stdint.h>

int8_t color = 1;
int8_t bgcolor = 0;
uint8_t timeForRedraw = 48;
extern uint8_t fixed_res_bit;
volatile uint8_t thiskey;
volatile uint16_t timers[8];
// Not implemented
/* #define USE_VIRTUAL_KBD */

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

void line(int x, int y, int x1, int y1) { drwLine(x, y, x1, y1); }

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
    printc(c);
    return c;
}

int puts(char *message) {
    int j = 0;
    while (*(message + j) != 0 && j <= 1000) {
        printc(*(message + j));
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
        printc(s_buffer[j]);
        j++;
    }
    return 0;
}

void printfix(int16_t value) {
    char sbuffer[10];
    const uint16_t fractPartMask = (1 << fixed_res_bit) - 1;
    int16_t j;
    if (value == 0) {
        printc('0');
    }
    if (value < 0) {
        printc('-');
        value = (~value) + 1;
    }
    int16_t intPart = value >> fixed_res_bit;
    value &= fractPartMask;

    // преобразуем целую часть
    itoa(intPart, sbuffer, 10);
    j = 0;
    while (sbuffer[j]) {
        printc(sbuffer[j]);
        j++;
    }
    char *ptr = sbuffer;
    // если есть дробная часть
    if (value != 0) {
        *ptr = '.';
        for (j = 0; j < 3; j++) {
            value &= fractPartMask;
            value *= 10;
            // value <<= 1;
            // value += value << 2;
            *++ptr = (uint8_t)(value >> fixed_res_bit) + '0';
        }
        // удаляем завершаюшие нули
        while (ptr[0] == '0')
            --ptr;
        ptr[1] = 0;
    }
    j = 0;
    while (sbuffer[j]) {
        printc(sbuffer[j]);
        j++;
    }
}

int printf(char *formatString, ...) {
    va_list args;
    va_start(args, formatString);

    while (*formatString) {
        if (*formatString == '%') {
            formatString++;
            switch (*formatString) {
            case '%':
                putchar('%');
                break;
            case 'd':
            case 'i':
                putn(va_arg(args, int));
                break;
            case 'c':
                putchar((char)va_arg(args, int));
                break;
            case 's':
                puts(va_arg(args, char *));
                break;
            case 'f':
                printfix((fixed)va_arg(args, int));
                break;
            default:
                puts("Unsupported format");
                va_end(args);
                return 1;
                break;
            }
        } else {
            putchar(*formatString); // 普通字符直接输出
        }
        formatString++; // 移动到下一个字符
    }
    va_end(args);
    return 0;
}

int getchar() {
#ifdef USE_VIRTUAL_KBD
    static char strBuf[16];
    static uint8_t strBufLength = 0;
    static uint8_t strBufPosition = 0;
    uint8_t virtualKeyboard(uint8_t kx, uint8_t ky, char buf[], uint8_t len);
    if (strBufLength == 0) {
        // So the virtual keyboard won't block input cursor
        if (char_y > 8) { // NOT-TODO: This is screen size dependent
            strBufLength = virtualKeyboard(2, 2, strBuf, sizeof(strBuf));
        } else {
            strBufLength = virtualKeyboard(2, 78, strBuf, sizeof(strBuf));
        }
        if (strBufLength == 0) { // User just input an `enter`
            strBuf[0] = '\n';
            strBufLength = 1;
        }
        setRedrawRect(0, 128);
        strBufPosition = 0;
    }
    if (strBufLength > 0) {
        if (strBufPosition < strBufLength) {
            strBufPosition++;
            return strBuf[strBufPosition];
        } else {
            strBufPosition = 0;
            strBufLength = 0;
            return 0; // NOT-TODO：
                      // 原版这里写得太混乱了。而且两个解释器行为不一致
            // 再考虑到应该不会有人用这里。以后再梳理吧。反正这个return
            // 0肯定是不对的
        }
    }
#endif
    printf("NOT implemented");
    return 0;
}

void loadfont(char *font, char first, char end) { fontload(font, first, end); }

void setfontsize(int picture_width, int picture_height, int character_width,
                 int character_height) {
    fontsize(picture_width, picture_height, character_width, character_height);
}

void drawstring(char *text, int x, int y) { drawString(text, x, y); }

void drawchar(char character, int x, int y) { drawChar(character, x, y); }

void loadtile(unsigned char **address, int imgwidth, int imgheight, int width,
              int height) {
    loadTile(address, imgwidth, imgheight, width, height);
}

void drawtile(int x, int y) { drawTile(x, y); }

void setcollisionmap(unsigned char *address) { setTileCollisionMap(address); }

unsigned char *gettileinxy(int x, int y) {
    return getTileInXY(x, y, (unsigned char *)0);
}

void setparticle(int gravity, int count, int time) {
    setParticle(gravity, count, time);
}

void setemitter(int time, int dir, int dir1, unsigned char speed) {
    setEmitter(time, dir, dir1, speed);
}

void emittersize(int width, int height, unsigned char size) {
    setEmitterSize(width, height, size);
}

void drawparticle(int x, int y, int color) { drawParticle(x, y, color); }

int getkey(void) {
    return thiskey; // this is set by other process in esp version engine
}
unsigned short gettimer(int n) { return timers[n & 0x7]; }

void settimer(int n, unsigned short time) { timers[n & 0x7] = time; }
int savedata(char *name, void *data, int count) {
    return ge0_port_savedata(name, data, count);
}

int loaddata(char *name, void *data) { return ge0_port_loaddata(name, data); }

void tone(int freq, int delay) { addTone(freq, delay); }

void loadrtttl(char *address, int isLoop) {
    setRtttlAddress(address);
    setRtttlLoop(isLoop);
}

void playrtttl() { setRtttlPlay(1); }

void pausertttl() { setRtttlPlay(0); }

void stoprtttl() { setRtttlPlay(2); }

fixed sin(short angle) { return fixed_sin(angle); }

fixed cos(short angle) { return fixed_cos(angle); }

static int16_t isqrt(int16_t n) {
    int g = 0x8000;
    int c = 0x8000;
    for (;;) {
        if (g * g > n) {
            g ^= c;
        }
        c >>= 1;
        if (c == 0) {
            return g;
        }
        g |= c;
    }
}

short distance(short x1, short y1, short x2, short y2) {
    return isqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

#ifdef MEMORY_BY_GE0
int *malloc(int size) { return ge0_port_malloc(size); }

void free(int *array) { ge0_port_free(array); }

void memcpy(int *array1, int *array2, int size) {
    ge0_port_memcpy(array1, array2, size);
}
#endif // MEMORY_BY_GE0
