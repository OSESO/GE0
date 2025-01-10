#include "screen.h"
#include "engine.h"
#include <stdint.h>

// This file includes functions to control the canvas in engine

#define SPRITE_IS_SOLID(a) (sprite_table[a].flags & 1)
#define SPRITE_IS_SCROLLED(a) (sprite_table[a].flags & 2)
#define SPRITE_IS_ONEBIT(a) (sprite_table[a].flags & 4)
#define SPRITE_IS_FLIP_HORIZONTAL(a) (sprite_table[a].flags & 8)
#define SET_LINE_IS_DRAW(a) line_is_draw[(a) >> 5] |= (1 << ((a) & 31))
#define PARTICLE_COUNT 32
#define SPRITE_COUNT 32
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
    uint8_t ibit;
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

int16_t getSpriteValue(uint16_t n, uint16_t t) {
    if (n > SPRITE_COUNT)
        return 0;
    switch (t) {
    case 0:
        return sprite_table[n].x >> 2;
    case 1:
        return sprite_table[n].y >> 2;
    case 2:
        return sprite_table[n].speedx;
    case 3:
        return sprite_table[n].speedy;
    case 4:
        return sprite_table[n].width;
    case 5:
        return sprite_table[n].height;
    case 6:
        return sprite_table[n].angle;
    case 7:
        return sprite_table[n].lives;
    case 8:
        return sprite_table[n].collision;
    case 9:
        return SPRITE_IS_SCROLLED(n); // todo：为什么这里是这个 9 是 s_solid才对
    case 10:
        return sprite_table[n].gravity;
    }
    return 0;
}
