#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <engine.h>
#include <stdint.h>

#define SCREEN_HEIGHT 128
#define SCREEN_WIDTH 128
#define SCREEN_WIDTH_BYTES 64

#define SCREEN_SIZE (SCREEN_HEIGHT * SCREEN_WIDTH_BYTES)
#define SCREEN_ARRAY_DEF SCREEN_SIZE

#define SCREEN_ADDR(x, y) (((y) << 6) + (x))

struct sprite {
    uint16_t address;
    int16_t x;
    int16_t y;
    int16_t previousx;
    int16_t previousy;
    uint8_t width;
    uint8_t height;
    uint16_t size;
    uint8_t zindex;
    int8_t speedx;
    int8_t speedy;
    int16_t angle;
    int8_t lives;
    int8_t collision;
    uint8_t flags; // 8 ~ 4 color 3 fliphorizontal 2 isonebit 1 scrolled 0 solid
    int8_t gravity;
    uint16_t oncollision;
    uint16_t onexitscreen;
};

struct Tile {
    int16_t adr;
    uint8_t imgwidth;
    uint8_t imgheight;
    uint8_t width;
    uint8_t height;
    int16_t x;
    int16_t y;
    uint16_t pixwidth;
    uint16_t pixheight;
    uint16_t collisionMap;
};

extern uint32_t line_is_draw[];
extern uint8_t screen[];
extern uint8_t sprite_screen[];
extern uint16_t palette[];
extern uint16_t sprtpalette[];

void clearScr(uint8_t p);
void changePalette(uint8_t n, uint16_t c);
void setPix(uint16_t x, uint16_t y, uint8_t p);
void setImageSize(uint16_t size);
void setClip(int16_t x0, int16_t y0, int16_t width, int16_t height);
void setCharX(int8_t x);
void setCharY(int8_t y);
void scrollScreen(uint8_t step, uint8_t direction);
void drawImg(uint8_t *image, int16_t x, int16_t y, int16_t w, int16_t h);
void drawImageBit(uint8_t *image, int16_t x1, int16_t y1, int16_t w, int16_t h);
void drawImgRLE(uint8_t *image, int16_t x1, int16_t y1, int16_t w, int16_t h);
int16_t getSpriteValue(uint16_t n, SpriteAttribute t);
void setSpriteValue(uint16_t n, SpriteAttribute t, int16_t v);
int16_t angleBetweenSprites(uint16_t n1, uint16_t n2);
void setSpr(uint16_t n, uint16_t adr);
void setSprPosition(uint16_t n, uint16_t x, uint16_t y);
void spriteSetDirectionAndSpeed(uint16_t n, uint16_t speed, int16_t dir);
int16_t getSpriteInXY(int16_t x, int16_t y);
uint8_t getPix(uint8_t x, uint8_t y);
void drwLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void drwRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void fllRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void drwTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                 uint16_t x3, uint16_t y3);
void fllTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                 uint16_t x2, uint16_t y2);
void drwCirc(int16_t x0, int16_t y0, int16_t r);
void fllCirc(int16_t x0, int16_t y0, int16_t r);
void printc(char c, uint8_t fc, uint8_t bc);
#endif
