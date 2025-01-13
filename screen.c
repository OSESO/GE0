#include "screen.h"
#include "engine.h"
#include "font_a.h"
#include <klib-macros.h>

// This file includes functions to control the canvas in engine

#define SPRITE_IS_SOLID(a) (sprite_table[a].flags & 1)
#define SPRITE_IS_SCROLLED(a) (sprite_table[a].flags & 2)
#define SPRITE_IS_ONEBIT(a) (sprite_table[a].flags & 4)
#define SPRITE_IS_FLIP_HORIZONTAL(a) (sprite_table[a].flags & 8)
#define SET_LINE_IS_DRAW(a) line_is_draw[(a) >> 5] |= (1 << ((a) & 31))
#define PARTICLE_COUNT 32
#define SPRITE_COUNT 32
#define MULTIPLY_FP_RESOLUTION_BITS 8
#define abs(x) ((x) > 0 ? (x) : -(x))
/* screen and sprite_screen
 * 7        4 3        0
 * ┌─────────┬─────────┐
 * │    0    │    1    │
 * └─────────┴─────────┘
 */
uint8_t
    screen[SCREEN_ARRAY_DEF * sizeof(uint8_t)]; // Each pixel usse 4 bit, which
                                                // point to a palette index
uint8_t sprite_screen[SCREEN_ARRAY_DEF * sizeof(uint8_t)];
uint32_t line_is_draw[4]; // Keep whether the line is changed since the last
                          // redraw Each bit represents one line
struct sprite sprite_table[SPRITE_COUNT];
// todo: change 'draw' to 'drawn'
// This engine uses rgb565 format palette
uint16_t palette[16];
uint16_t sprtpalette[16];
struct Tile tile;

int16_t imageSize = 1;
uint8_t clipx0 = 0;
uint8_t clipx1 = 128;
uint8_t clipy0 = 0;
uint8_t clipy1 = 128;
uint8_t isClip = 0;
int8_t regx = 0;
int8_t regy = 0;
char charArray[340];
extern int8_t bgcolor;
extern int8_t color;

const uint8_t fixed_res_bit = 8;

int16_t readInt(uint16_t adr);
uint8_t readMem(uint16_t adr);

// Changes palette of the pixel
void setPix(uint16_t x, uint16_t y, uint8_t p) {
    uint16_t x_index;  // Each unit in screen[] represents TWO pixel
    uint8_t orig, new; // This function changes ONE pixel at a time.
                       // orig is used to keep the other half bit
    if (0) {
        // todo :1484
    } else {
        if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
            x_index = x >> 1;
            orig = screen[SCREEN_ADDR(x_index, y)];
            if (x & 1)
                new = (orig & 0xf0) + p; // Keep the high-4 bits
            else
                new = (orig & 0x0f) + (p << 4); // Keep the low-4 bits
            if (orig != new) {                  // Only redraw if changed
                SET_LINE_IS_DRAW(y);
                screen[SCREEN_ADDR(x_index, y)] = new;
            }
        }
    }
}

void clearScr(uint8_t p) {
    for (uint16_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (uint16_t x = 0; x < SCREEN_WIDTH; x++) {
            setPix(x, y, p);
        }
    }
}

// changes the Palette colour denoted by n to the colour denoted by r5g6b5
// Palette 0-15 is for screen, 16-31 is for sprite
void changePalette(uint8_t n, uint16_t c) {
    if (n < 16) {
        palette[n] = c;
        for (uint8_t y = 0; y < 128; y++) {
            for (uint8_t x = 0; x < 64; x++) { // todo: Why x here is 64?
                if (((screen[SCREEN_ADDR(x, y)] & 0xf0) >> 4) == n ||
                    (screen[SCREEN_ADDR(x, y)] & 0x0f) == n)
                    SET_LINE_IS_DRAW(y);
            }
        }
    } else if (n < 32)
        sprtpalette[n - 16] = c;
}

void setImageSize(uint16_t size) { imageSize = size & 0x7fff; }

void setClip(int16_t x0, int16_t y0, int16_t width, int16_t height) {
    clipx0 = (x0 >= 0 && x0 < 127) ? x0 : 0;
    clipy0 = (y0 >= 0 && y0 < 127) ? y0 : 0;
    clipx1 = (x0 + width > 0 && x0 + width <= 127) ? x0 + width : 128;
    clipy1 = (y0 + height > 0 && y0 + height <= 127) ? y0 + height : 128;
    if (clipx0 == 0 && clipy0 == 0 && clipx1 == 128 && clipy1 == 128)
        isClip = 0;
    else
        isClip = 1;
}
void setCharX(int8_t x) { regx = x; }

void setCharY(int8_t y) { regy = y; }

void drawImgS(uint8_t *image, int16_t x, int16_t y, int32_t w, int32_t h) {
    uint32_t p, x2, y2, color, s, endx;
    s = imageSize;
    endx = ((w * s) >> fixed_res_bit);
    for (int32_t yi = 0; yi < ((h * s) >> fixed_res_bit); yi++) {
        y2 = ((yi << fixed_res_bit) + 1) / s;
        if ((y + yi) > 128)
            return;
        for (int32_t xi = 0; xi < endx; xi++) {
            x2 = ((xi << fixed_res_bit) + 1) / s;
            if (x2 & 1) {
                p = *(image + x2 / 2 + (y2 * w) / 2);
                color = (p & 0x0f);
            } else {
                p = *(image + x2 / 2 + (y2 * w) / 2);
                color = (p & 0xf0) >> 4;
            }
            if (color)
                setPix(x + xi, y + yi, color);
        }
    }
}

void fillRect(int8_t x, int8_t y, uint8_t w, uint8_t h, uint8_t c) {
    for (int16_t jx = x; jx < x + w; jx++)
        for (int16_t jy = y; jy < y + h; jy++)
            setPix(jx, jy, c);
}

void drawImg(uint8_t *image, int16_t x, int16_t y, int16_t w, int16_t h) {
    if (!(imageSize <= 1 || imageSize == (1 << fixed_res_bit))) {
        drawImgS(image, x, y, w, h);
        return;
    }
    uint8_t p, color;
    for (int yi = 0; yi < h; yi++)
        for (int xi = 0; xi < w; xi++) {
            p = *image;
            color = (p & 0xf0) >> 4;
            if (color > 0) {
                setPix(xi + x, yi + y, color);
            }
            xi++;
            color = p & 0x0f;
            if (color > 0) {
                setPix(xi + x, yi + y, color);
            }
            image++;
        }
}

void tileDrawLine(uint8_t step, uint8_t direction) {
    int16_t x, y, x0, y0, y1, nx, ny;
    uint16_t imgadr;
    if (direction == 2) {
        tile.x -= step * 2;
        x0 = tile.x;
        y0 = tile.y;
        x = (127 - x0) / tile.imgwidth;
        nx = x0 + x * tile.imgwidth;
        if (x < tile.width && x >= -tile.width) {
            for (y = 0; y < tile.height; y++) {
                ny = y0 + y * tile.imgheight;
                if (ny > -tile.height && ny < 128) {
                    // todo: 这里要修改tile.adr的存储方式
                    // 才能比较好地去除readInt
                    imgadr = readInt(tile.adr + (x + y * tile.width) * 2);
                    if (imgadr > 0)
                        drawImg(0 /*imgadr*/, nx, ny, tile.imgwidth,
                                tile.imgheight);
                    else
                        fillRect(nx, ny, tile.imgwidth, tile.imgheight,
                                 bgcolor);
                }
            }
        } else if (tile.width * tile.imgwidth + x0 >= 0) {
            y0 = (y0 > 0) ? y0 : 0;
            y1 = (tile.y + tile.height * tile.imgheight < 128)
                     ? tile.y + tile.height * tile.imgheight - y0
                     : 127 - y0;
            if (y0 < 127 && y1 > 0)
                fillRect(127 - step * 2, y0, step * 2, y1, bgcolor);
        }
    } else if (direction == 1) {
        tile.y -= step;
        x0 = tile.x;
        y0 = tile.y;
        y = (127 - y0) / tile.imgheight;
        ny = y0 + y * tile.imgheight;
        if (y < tile.height && y >= -tile.height)
            for (x = 0; x < tile.width; x++) {
                nx = x0 + x * tile.imgwidth;
                if (nx > -tile.width && nx < 128) {
                    imgadr = readInt(tile.adr + (x + y * tile.width) * 2);
                    if (imgadr > 0)
                        drawImg(0 /*imgadr*/, nx, ny, tile.imgwidth,
                                tile.imgheight);
                    else
                        fillRect(nx, ny, tile.imgwidth, tile.imgheight,
                                 bgcolor);
                }
            }
    } else if (direction == 0) {
        tile.x += step * 2;
        x0 = tile.x;
        y0 = tile.y;
        x = (0 - x0) / tile.imgwidth;
        nx = x0 + x * tile.imgwidth;
        if (x0 < 0 && x >= -tile.width) {
            for (y = 0; y < tile.height; y++) {
                ny = y0 + y * tile.imgheight;
                if (ny > -tile.height && ny < 128) {
                    imgadr = readInt(tile.adr + (x + y * tile.width) * 2);
                    if (imgadr > 0)
                        drawImg(0 /*imgadr*/, nx, ny, tile.imgwidth,
                                tile.imgheight);
                    else
                        fillRect(nx, ny, tile.imgwidth, tile.imgheight,
                                 bgcolor);
                }
            }
        } else if (x0 < 128) {
            y0 = (y0 > 0) ? y0 : 0;
            y1 = (tile.y + tile.height * tile.imgheight < 128)
                     ? tile.y + tile.height * tile.imgheight - y0
                     : 127 - y0;
            if (y0 < 127 && y1 > 0)
                fillRect(0, y0, step * 2, y1, bgcolor);
        }
    } else if (direction == 3) {
        tile.y += step;
        x0 = tile.x;
        y0 = tile.y;
        y = (0 - y0) / tile.imgheight;
        ny = y0 + y * tile.imgheight;
        if (y0 < 0 && y >= -tile.height)
            for (x = 0; x < tile.width; x++) {
                nx = x0 + x * tile.imgwidth;
                if (nx > -tile.width && nx < 128) {
                    imgadr = readInt(tile.adr + (x + y * tile.width) * 2);
                    if (imgadr > 0)
                        drawImg(0 /*imgadr*/, nx, ny, tile.imgwidth,
                                tile.imgheight);
                    else
                        fillRect(nx, ny, tile.imgwidth, tile.imgheight,
                                 bgcolor);
                }
            }
    }
}

void scrollScreen(uint8_t step, uint8_t direction) {
    uint8_t bufPixel;
    if (direction == 2) {
        for (uint8_t y = clipy0; y < clipy1; y++) {
            bufPixel = screen[SCREEN_ADDR(clipx0 / 2, y)];
            for (uint8_t x = clipx0 / 2 + 1; x < clipx1 / 2; x++) {
                if (screen[SCREEN_ADDR(x - 1, y)] != screen[SCREEN_ADDR(x, y)])
                    SET_LINE_IS_DRAW(y);
                screen[SCREEN_ADDR(x - 1, y)] = screen[SCREEN_ADDR(x, y)];
            }
            if (screen[SCREEN_ADDR((clipx1 - 1) / 2, y)] != bufPixel)
                SET_LINE_IS_DRAW(y);
            screen[SCREEN_ADDR((clipx1 - 1) / 2, y)] = bufPixel;
        }
        for (uint8_t n = 0; n < 32; n++)
            if (SPRITE_IS_SCROLLED(n) && !isClip) {
                sprite_table[n].x -= 8;
                sprite_table[n].previousx -= 8;
            }
    } else if (direction == 1) {
        for (uint8_t x = clipx0 / 2; x < clipx1 / 2; x++) {
            bufPixel = screen[SCREEN_ADDR(x, clipy0)];
            for (uint8_t y = clipy0 + 1; y < clipy1; y++) {
                if (screen[SCREEN_ADDR(x, y - 1)] != screen[SCREEN_ADDR(x, y)])
                    SET_LINE_IS_DRAW(y);
                screen[SCREEN_ADDR(x, y - 1)] = screen[SCREEN_ADDR(x, y)];
            }
            if (screen[SCREEN_ADDR(x, clipy1 - 1)] != bufPixel)
                SET_LINE_IS_DRAW(clipy1 - 1);
            screen[SCREEN_ADDR(x, clipy1 - 1)] = bufPixel;
        }
        for (uint8_t n = 0; n < 32; n++)
            if (SPRITE_IS_SCROLLED(n) && !isClip) {
                sprite_table[n].y -= 4;
                sprite_table[n].previousy -= 4;
            }
    } else if (direction == 0) {
        for (uint8_t y = clipy0; y < clipy1; y++) {
            bufPixel = screen[SCREEN_ADDR((clipx1 - 1) / 2, y)];
            for (uint8_t x = (clipx1 - 1) / 2; x > clipx0 / 2; x--) {
                if (screen[SCREEN_ADDR(x, y)] != screen[SCREEN_ADDR(x - 1, y)])
                    SET_LINE_IS_DRAW(y);
                screen[SCREEN_ADDR(x, y)] = screen[SCREEN_ADDR(x - 1, y)];
            }
            if (screen[SCREEN_ADDR(clipx0 / 2, y)] != bufPixel)
                SET_LINE_IS_DRAW(y);
            screen[SCREEN_ADDR(clipx0 / 2, y)] = bufPixel;
        }
        for (uint8_t n = 0; n < 32; n++)
            if (SPRITE_IS_SCROLLED(n) && !isClip) {
                sprite_table[n].x += 8;
                sprite_table[n].previousx += 8;
            }
    } else {
        for (uint8_t x = clipx0 / 2; x < clipx1 / 2; x++) {
            bufPixel = screen[SCREEN_ADDR(x, (clipx1 - 1) / 2)];
            for (uint8_t y = clipy1 - 1; y > clipy0; y--) {
                if (screen[SCREEN_ADDR(x, y)] != screen[SCREEN_ADDR(x, y - 1)])
                    SET_LINE_IS_DRAW(y);
                screen[SCREEN_ADDR(x, y)] = screen[SCREEN_ADDR(x, y - 1)];
            }
            if (screen[SCREEN_ADDR(x, clipy0)] != bufPixel)
                SET_LINE_IS_DRAW(0);
            screen[SCREEN_ADDR(x, clipy0)] = bufPixel;
        }
        for (uint8_t n = 0; n < 32; n++)
            if (SPRITE_IS_SCROLLED(n) && !isClip) {
                sprite_table[n].y += 4;
                sprite_table[n].previousy += 4;
            }
    }
    if (tile.adr > 0 && !isClip)
        tileDrawLine(step, direction);
}
void drawImageBitS(uint8_t *img, int16_t x, int16_t y, int16_t w, int16_t h) {
    uint32_t p, x2, y2, s;
    s = imageSize;
    for (int32_t yi = 0; yi < ((h * s) >> fixed_res_bit); yi++) {
        y2 = ((yi << fixed_res_bit) + 1) / s;
        if ((y + yi) > 128)
            return;
        for (int32_t xi = 0; xi < ((w * s) >> fixed_res_bit); xi++) {
            x2 = ((xi << fixed_res_bit) + 1) / s;
            p = *(img + (x2 + y2 * w) / 8);
            if (p & (1 << (7 - ((x2 + y2 * w) & 7))))
                setPix(x + xi, y + yi, color);
            else
                setPix(x + xi, y + yi, bgcolor);
        }
    }
}

void drawImageBit(uint8_t *image, int16_t x1, int16_t y1, int16_t w,
                  int16_t h) {
    if (!(imageSize <= 1 || imageSize == (1 << fixed_res_bit))) {
        drawImageBitS(image, x1, y1, w, h);
        return;
    }
    int16_t i = 0;
    uint8_t ibit = 0;
    for (int16_t y = 0; y < h; y++)
        for (int16_t x = 0; x < w; x++) {
            if (i % 8 == 0) {
                ibit = *(image);
                image++;
            }
            if (ibit & 0x80)
                setPix(x1 + x, y1 + y, color);
            else
                setPix(x1 + x, y1 + y, bgcolor);
            ibit = ibit << 1;
            i++;
        }
}

void drawImgRLES(uint8_t *image, int16_t x1, int16_t y1, int16_t w, int16_t h) {
    int16_t i = 0;
    uint8_t jx, jy;
    uint8_t repeat = *(image);
    image++;
    int8_t color1 = (*(image) & 0xf0) >> 4;
    int8_t color2 = *(image) & 0xf;
    uint8_t s = imageSize >> fixed_res_bit;
    while (i < w * h) {
        if (repeat > 0x81) {
            if (color1 > 0) {
                for (jx = 0; jx < s; jx++)
                    for (jy = 0; jy < s; jy++)
                        setPix(x1 + (i % w) * s + jx, y1 + i / w * s + jy,
                               color1);
            }
            if (color2 > 0) {
                for (jx = 0; jx < s; jx++)
                    for (jy = 0; jy < s; jy++)
                        setPix(x1 + (i % w) * s + s + jx, y1 + i / w * s + jy,
                               color2);
            }
            i += 2;
            image++;
            repeat--;
            color1 = (*(image) & 0xf0) >> 4;
            color2 = *(image) & 0xf;
        } else if (repeat == 0x81) {
            repeat = *(image);
            image++;
            color1 = (*(image) & 0xf0) >> 4;
            color2 = *(image) & 0xf;
        } else if (repeat > 0) {
            if (color1 > 0) {
                for (jx = 0; jx < s; jx++)
                    for (jy = 0; jy < s; jy++)
                        setPix(x1 + (i % w) * s + jx, y1 + i / w * s + jy,
                               color1);
            }
            if (color2 > 0) {
                for (jx = 0; jx < s; jx++)
                    for (jy = 0; jy < s; jy++)
                        setPix(x1 + (i % w) * s + s + jx, y1 + i / w * s + jy,
                               color2);
            }
            i += 2;
            repeat--;
        } else if (repeat == 0) {
            image++;
            repeat = *(image);
            image++;
            color1 = (*(image) & 0xf0) >> 4;
            color2 = *(image) & 0xf;
        }
    }
}

void drawImgRLE(uint8_t *image, int16_t x1, int16_t y1, int16_t w, int16_t h) {
    if (!(imageSize <= 1 || imageSize == (1 << fixed_res_bit))) {
        drawImgRLES(image, x1, y1, w, h);
        return;
    }
    int16_t i = 0;
    uint8_t repeat = *(image);
    image++;
    int8_t color1 = (*(image) & 0xf0) >> 4;
    int8_t color2 = *(image) & 0xf;
    while (i < w * h) {
        if (repeat > 0x81) {
            if (color1 > 0) {
                setPix(x1 + i % w, y1 + i / w, color1);
            }
            if (color2 > 0) {
                setPix(x1 + i % w + 1, y1 + i / w, color2);
            }
            i += 2;
            image++;
            repeat--;
            color1 = (*(image) & 0xf0) >> 4;
            color2 = *(image) & 0xf;
        } else if (repeat == 0x81) {
            repeat = *(image);
            image++;
            color1 = (*(image) & 0xf0) >> 4;
            color2 = *(image) & 0xf;
        } else if (repeat > 0) {
            if (color1 > 0) {
                setPix(x1 + i % w, y1 + i / w, color1);
            }
            if (color2 > 0) {
                setPix(x1 + i % w + 1, y1 + i / w, color2);
            }
            i += 2;
            repeat--;
        } else if (repeat == 0) {
            image++;
            repeat = *(image);
            image++;
            color1 = (*(image) & 0xf0) >> 4;
            color2 = *(image) & 0xf;
        }
    }
}

int16_t getSpriteValue(uint16_t n, SpriteAttribute t) {
    if (n >= SPRITE_COUNT)
        return 0;
    switch (t) {
    case S_X:
        return sprite_table[n].x >> 2;
    case S_Y:
        return sprite_table[n].y >> 2;
    case S_SPEEDX:
        return sprite_table[n].speedx;
    case S_SPEEDY:
        return sprite_table[n].speedy;
    case S_WIDTH:
        return sprite_table[n].width;
    case S_HEIGHT:
        return sprite_table[n].height;
    case S_ANGLE:
        return sprite_table[n].angle;
    case S_LIVES:
        return sprite_table[n].lives;
    case S_COLLISION:
        return sprite_table[n].collision;
    case S_SOLID:
        return SPRITE_IS_SOLID(n);
    case S_GRAVITY:
        return sprite_table[n].gravity;
    default: // TODO: too lazy to add the remainings now
        return 0;
    }
    return 0;
}

void setSpriteValue(uint16_t n, SpriteAttribute t, int16_t v) {
    if (n >= SPRITE_COUNT)
        return;
    switch (t) {
    case S_X:
        sprite_table[n].x = v << 2;
        return;
    case S_Y:
        sprite_table[n].y = v << 2;
        return;
    case S_SPEEDX:
        sprite_table[n].speedx = (int8_t)v;
        return;
    case S_SPEEDY:
        sprite_table[n].speedy = (int8_t)v;
        return;
    case S_WIDTH:
        sprite_table[n].width = v;
        return;
    case S_HEIGHT:
        sprite_table[n].height = v;
        return;
    case S_ANGLE:
        v = v % 360;
        if (v < 0)
            v += 360;
        sprite_table[n].angle = v;
        return;
    case S_LIVES:
        sprite_table[n].lives = v;
        return;
    case S_COLLISION:
        return;
    case S_SOLID:
        if (v != 0)
            sprite_table[n].flags |= 0x01;
        else
            sprite_table[n].flags &= ~0x01;
        return;
    case S_GRAVITY:
        sprite_table[n].gravity = v;
        return;
    case S_ON_COLLISION:
        sprite_table[n].oncollision = (uint16_t)v;
        return;
    case S_ON_EXIT_SCREEN:
        sprite_table[n].onexitscreen = (uint16_t)v;
        return;
    case S_IS_SCROLLED:
        if (v != 0)
            sprite_table[n].flags |= 0x02;
        else
            sprite_table[n].flags &= ~0x02;
        return;
    case S_IS_ONEBIT:
        if (v != 0)
            sprite_table[n].flags |= 0x04;
        else
            sprite_table[n].flags &= ~0x04;
        return;
    case S_FLIP_HORIZONTAL: // TODO: where does this 15 16 17 comes from
        if (v != 0)
            sprite_table[n].flags |= 0x08;
        else
            sprite_table[n].flags &= ~0x08;
        return;
    case S_Z_INDEX:
        sprite_table[n].zindex = (uint8_t)v;
        return;
    case S_COLOR:
        sprite_table[n].flags &= 0x0f;
        sprite_table[n].flags |= (uint8_t)v << 4;
        return;
    }
}
int16_t atan2_fp(int16_t y_fp, int16_t x_fp) {
    int32_t coeff_1 = 45;
    int32_t coeff_1b = -56; // 56.24;
    int32_t coeff_1c = 11;  // 11.25
    int16_t coeff_2 = 135;
    int16_t angle = 0;
    int32_t r;
    int32_t r3;
    int16_t y_abs_fp = y_fp;
    if (y_abs_fp < 0)
        y_abs_fp = -y_abs_fp;
    if (y_fp == 0) {
        if (x_fp >= 0) {
            angle = 0;
        } else {
            angle = 180;
        }
    } else if (x_fp >= 0) {
        r = (((int32_t)(x_fp - y_abs_fp)) << MULTIPLY_FP_RESOLUTION_BITS) /
            ((int32_t)(x_fp + y_abs_fp));
        r3 = r * r;
        r3 = r3 >> MULTIPLY_FP_RESOLUTION_BITS;
        r3 *= r;
        r3 = r3 >> MULTIPLY_FP_RESOLUTION_BITS;
        r3 *= coeff_1c;
        angle = (int16_t)(coeff_1 +
                          ((coeff_1b * r + r3) >> MULTIPLY_FP_RESOLUTION_BITS));
    } else {
        r = (((int32_t)(x_fp + y_abs_fp)) << MULTIPLY_FP_RESOLUTION_BITS) /
            ((int32_t)(y_abs_fp - x_fp));
        r3 = r * r;
        r3 = r3 >> MULTIPLY_FP_RESOLUTION_BITS;
        r3 *= r;
        r3 = r3 >> MULTIPLY_FP_RESOLUTION_BITS;
        r3 *= coeff_1c;
        angle =
            coeff_2 +
            ((int16_t)(((coeff_1b * r + r3) >> MULTIPLY_FP_RESOLUTION_BITS)));
    }
    if (y_fp < 0)
        return (-angle); // negate if in quad III or IV
    else
        return (angle);
}
int16_t fixed_sin(int x) {
    // Bhaskara I's sine approximation sin(x°) = 4·x·(180−x)/(40500−x·(180−x))
    char pos = 1; // positive - keeps an eye on the sign.
    if (x < 0) {
        x = -x;
        pos = !pos;
    }
    if (x >= 360)
        x %= 360;
    if (x > 180) {
        x -= 180;
        pos = !pos;
    }
    int16_t nv = x * (180 - x);
    int32_t s = (nv * 4 * (1 << fixed_res_bit)) / (40500 - nv);
    if (pos)
        return (int16_t)s;
    return (int16_t)-s;
}
int16_t fixed_cos(int16_t g) { return fixed_sin(g + 90); }

int16_t angleBetweenSprites(uint16_t n1, uint16_t n2) {
    if (n1 >= SPRITE_COUNT || n2 >= SPRITE_COUNT)
        return 0;
    int16_t A = atan2_fp(sprite_table[n1].y - sprite_table[n2].y,
                         sprite_table[n1].x - sprite_table[n2].x);
    A = (A < 0) ? A + 360 : A;
    return A;
}
void setSpr(uint16_t n, uint16_t adr) {
    if (n >= SPRITE_COUNT)
        return;
    sprite_table[n].address = adr;
}

void setSprPosition(uint16_t n, uint16_t x, uint16_t y) {
    if (n >= SPRITE_COUNT)
        return;
    sprite_table[n].x = x << 2;
    sprite_table[n].y = y << 2;
    sprite_table[n].previousx = x << 2;
    sprite_table[n].previousy = y << 2;
}

void spriteSetDirectionAndSpeed(uint16_t n, uint16_t speed, int16_t dir) {
    if (n >= SPRITE_COUNT)
        return;
    dir = dir % 360;
    if (dir < 0)
        dir += 360;
    sprite_table[n].speedx = ((speed * fixed_cos(dir)) >> fixed_res_bit);
    sprite_table[n].speedy = ((speed * fixed_sin(dir)) >> fixed_res_bit);
}

int16_t getSpriteInXY(int16_t x, int16_t y) {
    for (int n = 0; n < SPRITE_COUNT; n++) {
        if (sprite_table[n].lives > 0)
            if ((sprite_table[n].x >> 2) < x &&
                (sprite_table[n].x >> 2) + sprite_table[n].width > x &&
                (sprite_table[n].y >> 2) < y &&
                (sprite_table[n].y >> 2) + sprite_table[n].height > y)
                return n;
    }
    return -1;
}
uint8_t getPix(uint8_t x, uint8_t y) {
    uint16_t xi, b;
    if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
        xi = x / 2;
        if (x % 2)
            b = (screen[SCREEN_ADDR(xi, y)] & 0x0f);
        else
            b = (screen[SCREEN_ADDR(xi, y)] & 0xf0) >> 4;
        return b;
    }
    return 0;
}

void drawFVLine(int x, int y1, int y2) {
    for (int i = y1; i <= y2; i++)
        setPix(x, i, color);
}
void drawFHLine(int16_t x1, int16_t x2, uint16_t y) {
    uint8_t *nPtr, c;
    uint16_t i;
    if (isClip) {
        if (y < clipy0 || y >= clipy1)
            return;
        if (x1 < clipx0)
            x1 = clipx0;
        if (x2 >= clipx1)
            x2 = clipx1;
    } else {
        if (y > 127)
            return;
        if (x1 < 0)
            x1 = 0;
        if (x2 >= 127)
            x2 = 127;
    }
    if (x1 & 1) {
        screen[SCREEN_ADDR(x1 / 2, y)] =
            (screen[SCREEN_ADDR(x1 / 2, y)] & 0xf0) + color;
        x1++;
    }
    if (!(x2 & 1)) {
        screen[SCREEN_ADDR(x2 / 2, y)] =
            (screen[SCREEN_ADDR(x2 / 2, y)] & 0x0f) + (color << 4);
        x2--;
    }
    SET_LINE_IS_DRAW(y);
    i = x1 / 2;
    c = (color << 4) + color;
    nPtr = (uint8_t *)&screen[SCREEN_ADDR(i, y)];
    for (; i <= x2 / 2; i++)
        *nPtr++ = c;
}
void drwLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    if (x1 == x2) {
        if (y1 > y2)
            drawFVLine(x1, y2, y1);
        else
            drawFVLine(x1, y1, y2);
        return;
    } else if (y1 == y2) {
        if (x1 > x2)
            drawFHLine(x2, x1, y1);
        else
            drawFHLine(x1, x2, y1);
        return;
    }
    int deltaX = abs(x2 - x1);
    int deltaY = abs(y2 - y1);
    int signX = x1 < x2 ? 1 : -1;
    int signY = y1 < y2 ? 1 : -1;
    int error = deltaX - deltaY;
    int error2;
    setPix(x2, y2, color);
    while (x1 != x2 || y1 != y2) {
        setPix(x1, y1, color);
        error2 = error * 2;
        if (error2 > -deltaY) {
            error -= deltaY;
            x1 += signX;
        }
        if (error2 < deltaX) {
            error += deltaX;
            y1 += signY;
        }
    }
}

void drwRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    drawFHLine(x0, x1, y0);
    drawFHLine(x0, x1, y1);
    drawFVLine(x0, y0, y1);
    drawFVLine(x1, y0, y1);
}
void fllRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    for (int16_t jy = y0; jy <= y1; jy++)
        drawFHLine(x0, x1, jy);
}

void drwTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                 uint16_t x3, uint16_t y3) {
    drwLine(x1, y1, x2, y2);
    drwLine(x2, y2, x3, y3);
    drwLine(x3, y3, x1, y1);
}
void fllTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                 uint16_t x2, uint16_t y2) {
    int32_t a, b, y, last, t;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        t = y0;
        y0 = y1;
        y1 = t;
        t = x0;
        x0 = x1;
        x1 = t;
    }
    if (y1 > y2) {
        t = y2;
        y2 = y1;
        y1 = t;
        t = x2;
        x2 = x1;
        x1 = t;
    }
    if (y0 > y1) {
        t = y0;
        y0 = y1;
        y1 = t;
        t = x0;
        x0 = x1;
        x1 = t;
    }

    if (y0 == y2) {
        a = b = x0;
        if (x1 < a)
            a = x1;
        else if (x1 > b)
            b = x1;
        if (x2 < a)
            a = x2;
        else if (x2 > b)
            b = x2;
        drawFHLine(a, b, y0);
        return;
    }

    int32_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
            dx12 = x2 - x1, dy12 = y2 - y1, sa = 0, sb = 0;

    if (y1 == y2)
        last = y1;
    else
        last = y1 - 1;

    for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;

        if (a > b) {
            t = a;
            a = b;
            b = t;
        }
        drawFHLine(a, b, y);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for (; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;

        if (a > b) {
            t = a;
            a = b;
            b = t;
        }
        drawFHLine(a, b, y);
    }
}
void drwCirc(int16_t x0, int16_t y0, int16_t r) {
    int16_t x = 0;
    int16_t dx = 1;
    int16_t dy = r + r;
    int16_t p = -(r >> 1);

    // These are ordered to minimise coordinate changes in x or y
    // drawPixel can then send fewer bounding box commands
    setPix(x0 + r, y0, color);
    setPix(x0 - r, y0, color);
    setPix(x0, y0 - r, color);
    setPix(x0, y0 + r, color);

    while (x < r) {

        if (p >= 0) {
            dy -= 2;
            p -= dy;
            r--;
        }

        dx += 2;
        p += dx;

        x++;

        // These are ordered to minimise coordinate changes in x or y
        // drawPixel can then send fewer bounding box commands
        setPix(x0 + x, y0 + r, color);
        setPix(x0 - x, y0 + r, color);
        setPix(x0 - x, y0 - r, color);
        setPix(x0 + x, y0 - r, color);

        setPix(x0 + r, y0 + x, color);
        setPix(x0 - r, y0 + x, color);
        setPix(x0 - r, y0 - x, color);
        setPix(x0 + r, y0 - x, color);
    }
}
void fllCirc(int16_t x0, int16_t y0, int16_t r) {
    int16_t x = 0;
    int16_t dx = 1;
    int16_t dy = r + r;
    int16_t p = -(r >> 1);

    drawFHLine(x0 - r, x0 + r, y0);

    while (x < r) {
        if (p >= 0) {
            dy -= 2;
            p -= dy;
            r--;
        }

        dx += 2;
        p += dx;

        x++;

        drawFHLine(x0 - r, x0 + r, y0 + x);
        drawFHLine(x0 - r, x0 + r, y0 - x);
        drawFHLine(x0 - x, x0 + x, y0 + r);
        drawFHLine(x0 - x, x0 + x, y0 - r);
    }
}

void drawChar(char c, uint8_t x, uint8_t y) {
    for (int8_t i = 0; i < 5; i++) { // Char bitmap = 5 columns
        uint8_t line = (font_a[c * 5 + i]);
        for (int8_t j = 0; j < 8; j++, line >>= 1) {
            if (line & 1)
                setPix(x + i, y + j, color);
        }
    }
}
void charLineUp(uint8_t n) {
    clearScr(bgcolor);
    for (uint16_t i = 0; i < 336 - n * 21; i++) {
        charArray[i] = charArray[i + n * 21];
        drawChar(charArray[i], (i % 21) * 6, (i / 21) * 8);
    }
}

inline int8_t getCharY() { return regy; }

void printc(char c, uint8_t fc, uint8_t bc) {
    if (regy > 15) {
        regy = 15;
        charLineUp(1);
    }
    if (c == '\n') {
        fillRect(regx * 6, regy * 8, 127 - regx * 6, 8, bgcolor);
        for (uint8_t i = regx; i <= 21; i++) {
            charArray[regx + regy * 21] = ' ';
        }
        regy++;
        regx = 0;
    } else if (c == '\t') {
        for (uint8_t i = 0; i <= regx % 5; i++) {
            fillRect(regx * 6, regy * 8, 6, 8, bgcolor);
            charArray[regx + regy * 21] = ' ';
            regx++;
            if (regx > 20) {
                regy++;
                regx = 0;
            }
        }
    } else {
        fillRect(regx * 6, regy * 8, 6, 8, bgcolor);
        drawChar(c, regx * 6, regy * 8);
        charArray[regx + regy * 21] = c;
        regx++;
        if (regx > 20) {
            regy++;
            regx = 0;
        }
    }
}
