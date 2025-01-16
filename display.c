#include "display.h"
#include "ge0_port_interface.h"
#include "screen.h"
#include <stdbool.h>
#include <stddef.h>

// This file includes actual logic to draw on hardware

uint16_t rscreenWidth, rscreenHeight; // Real screen size
uint16_t displayXOffset;              // Margin of the display area
uint32_t pix_buffer[SCREEN_REAL_WIDTH];
uint8_t timeForRedraw = 48;
extern uint8_t fixed_res_bit;
void setScreenResolution(uint16_t nw, uint16_t nh) {
    if (nw < SCREEN_REAL_WIDTH)
        rscreenWidth = nw;
    else
        rscreenWidth = SCREEN_REAL_WIDTH - 1;

    if (nh < SCREEN_REAL_HEIGHT)
        rscreenHeight = nh;
    else
        rscreenHeight = SCREEN_REAL_HEIGHT - 1;

    if (rscreenWidth < 95)
        rscreenWidth = 95;
    if (rscreenHeight < 95)
        rscreenHeight = 95;

    displayXOffset = (SCREEN_REAL_WIDTH - rscreenWidth) / 2;
    for (int i = 0; i < 4; i++)
        line_is_draw[i] = 0xffffffff;
    ge0_port_display_fillScreen(0x0000);
}
void drawSprPixel(int8_t pixel, int16_t x0, int16_t y0, int16_t x, int16_t y) {
    if (x0 + x >= 0 && x0 + x < 128 && y0 + y >= 0 && y0 + y < 128) {
        if ((x0 + x) & 1)
            sprite_screen[SCREEN_ADDR((x0 + x) / 2, y0 + y)] =
                (sprite_screen[SCREEN_ADDR((x0 + x) / 2, y0 + y)] & 0xf0) +
                pixel;
        else
            sprite_screen[SCREEN_ADDR((x0 + x) / 2, y0 + y)] =
                (sprite_screen[SCREEN_ADDR((x0 + x) / 2, y0 + y)] & 0x0f) +
                (pixel << 4);
        SET_LINE_IS_DRAW(y0 + y);
    }
}
void largeParticle(int16_t x0, int16_t y0, int16_t r, int8_t c) {
    int16_t x = 0;
    int16_t dx = 1;
    int16_t dy = r + r;
    int16_t p = -(r >> 1);

    drawSprFHLine(x0 - r, x0 + r, y0, c);

    while (x < r) {
        if (p >= 0) {
            dy -= 2;
            p -= dy;
            r--;
        }

        dx += 2;
        p += dx;

        x++;

        drawSprFHLine(x0 - r, x0 + r, y0 + x, c);
        drawSprFHLine(x0 - r, x0 + r, y0 - x, c);
        drawSprFHLine(x0 - x, x0 + x, y0 + r, c);
        drawSprFHLine(x0 - x, x0 + x, y0 - r, c);
    }
}

void redrawParticles() {
    uint16_t n;
    uint16_t x, y;
    if (emitter.timer > 0) {
        emitter.timer -= 50;
        updateEmitter();
    }
    for (n = 0; n < PARTICLE_COUNT; n++)
        if (particles[n].time > 0) {
            x = ((particles[n].x >> 1) & 127);
            y = (particles[n].y >> 1) & 127;
            if (particles[n].size) {
                largeParticle(x, y, particles[n].size, particles[n].color);
            } else {
                x = x >> 1;
                if (particles[n].x & 1)
                    sprite_screen[SCREEN_ADDR(x, y)] =
                        (sprite_screen[SCREEN_ADDR(x, y)] & 0xf0) +
                        (particles[n].color & 0x0f);
                else
                    sprite_screen[SCREEN_ADDR(x, y)] =
                        (sprite_screen[SCREEN_ADDR(x, y)] & 0x0f) +
                        ((particles[n].color & 0x0f) << 4);
                SET_LINE_IS_DRAW(y);
            }
            particles[n].time -= 50;
            if (ge0_port_random_min_max(0, 2)) {
                particles[n].x += particles[n].speedx;
                particles[n].speedy += particles[n].gravity;
                particles[n].y += particles[n].speedy;
            } else {
                particles[n].x += particles[n].speedx / 2;
                particles[n].y += particles[n].speedy / 2;
            }
            if (particles[n].x < 0 || particles[n].x > 256 ||
                particles[n].y < 0 || particles[n].y > 256)
                particles[n].time = 0;
        }
}
void drawSprFHLine(int16_t x1, int16_t x2, int16_t y, int8_t c) {
    for (int i = x1; i <= x2; i++)
        drawSprPixel(c, i, y, 0, 0);
}

void drawRotateSprPixel(int8_t pixel, int16_t x0, int16_t y0, int16_t x,
                        int16_t y, int16_t hw, int16_t hh, int16_t c,
                        int16_t s) {

    int16_t nx = hw + (((x - hw) * c - (y - hh) * s) >> fixed_res_bit);
    int16_t ny = hh + (((y - hh) * c + (x - hw) * s) >> fixed_res_bit);
    int16_t nnx = nx / 2;
    int16_t nnx0 = x0 / 2;
    if (nnx0 + nnx >= 0 && nnx0 + nnx < 64 && y0 + ny >= 0 && y0 + ny < 128) {
        if (nx & 1)
            sprite_screen[SCREEN_ADDR(nnx0 + nnx, y0 + ny)] =
                (sprite_screen[SCREEN_ADDR(nnx0 + nnx, y0 + ny)] & 0xf0) +
                pixel;
        else
            sprite_screen[SCREEN_ADDR(nnx0 + nnx, y0 + ny)] =
                (sprite_screen[SCREEN_ADDR(nnx0 + nnx, y0 + ny)] & 0x0f) +
                (pixel << 4);
        SET_LINE_IS_DRAW(y0 + ny);
    }
}
void drawSpr(int16_t n, int16_t x, int16_t y) {
    char *adr = sprite_table[n].address;
    uint16_t w = sprite_table[n].width;
    uint16_t h = sprite_table[n].height;
    uint16_t ww = w;
    int16_t c, s;
    uint16_t sz, x1, y1, x2, y2, endx, endy, i;
    uint8_t pixel, ibit = 0;
    w = w / 2;
    sz = sprite_table[n].size;
    if (!SPRITE_IS_ONEBIT(n)) {
        if (!sprite_table[n].angle) {
            if (SPRITE_IS_FLIP_HORIZONTAL(n)) {
                if (sz != 1 << fixed_res_bit) {
                    endx = ((ww * sz) >> fixed_res_bit);
                    for (y1 = 0; y1 < ((h * sz) >> fixed_res_bit); y1++) {
                        y2 = ((y1 << fixed_res_bit) + 1) / sz;
                        if ((y + y1) > 128)
                            return;
                        for (x1 = 0; x1 < endx; x1++) {
                            x2 = ((x1 << fixed_res_bit) + 1) / sz;
                            pixel = *(adr + x2 / 2 + (y2 * ww) / 2);
                            if (x2 & 1) {
                                pixel = (pixel & 0x0f);
                            } else {
                                pixel = (pixel & 0xf0) >> 4;
                            }
                            if (pixel)
                                drawSprPixel(pixel, x, y, endx - x1, y1);
                        }
                    }
                } else {
                    for (y1 = 0; y1 < h; y1++) {
                        if (y1 + y >= -h && y1 + y < 128 + h) {
                            for (x1 = 0; x1 < w; x1++) {
                                pixel = *(adr + x1 + y1 * w);
                                if ((pixel & 0xf0) > 0)
                                    drawSprPixel(pixel >> 4, x, y, ww - x1 * 2,
                                                 y1);
                                if ((pixel & 0x0f) > 0)
                                    drawSprPixel(pixel & 0xf, x, y,
                                                 ww - (x1 * 2 + 1), y1);
                            }
                        }
                    }
                }
            } else {
                if (sz != 1 << fixed_res_bit) {
                    endx = ((ww * sz) >> fixed_res_bit);
                    for (y1 = 0; y1 < ((h * sz) >> fixed_res_bit); y1++) {
                        y2 = ((y1 << fixed_res_bit) + 1) / sz;
                        if ((y + y1) > 128)
                            return;
                        for (x1 = 0; x1 < endx; x1++) {
                            x2 = ((x1 << fixed_res_bit) + 1) / sz;
                            pixel = *(adr + x2 / 2 + (y2 * ww) / 2);
                            if (x2 & 1) {
                                pixel = (pixel & 0x0f);
                            } else {
                                pixel = (pixel & 0xf0) >> 4;
                            }
                            if (pixel)
                                drawSprPixel(pixel, x, y, x1, y1);
                        }
                    }
                } else {
                    for (y1 = 0; y1 < h; y1++) {
                        if (y1 + y >= -h && y1 + y < 128 + h) {
                            for (x1 = 0; x1 < w; x1++) {
                                pixel = *(adr + x1 + y1 * w);
                                if ((pixel & 0xf0) > 0)
                                    drawSprPixel(pixel >> 4, x, y, x1 * 2, y1);
                                if ((pixel & 0x0f) > 0)
                                    drawSprPixel(pixel & 0xf, x, y, x1 * 2 + 1,
                                                 y1);
                            }
                        }
                    }
                }
            }
        } else {
            c = fixed_cos(sprite_table[n].angle);
            s = fixed_sin(sprite_table[n].angle);
            if (SPRITE_IS_FLIP_HORIZONTAL(n)) {
                if (sz != 1 << fixed_res_bit) {
                    endx = ((ww * sz) >> fixed_res_bit);
                    endy = ((h * sz / 2) >> fixed_res_bit);
                    for (y1 = 0; y1 < ((h * sz) >> fixed_res_bit); y1++) {
                        y2 = ((y1 << fixed_res_bit) + 1) / sz;
                        if ((y + y1) > 128)
                            return;
                        for (x1 = 0; x1 < endx; x1++) {
                            x2 = ((x1 << fixed_res_bit) + 1) / sz;
                            pixel = *(adr + x2 / 2 + (y2 * ww) / 2);
                            if (x2 & 1) {
                                pixel = (pixel & 0x0f);
                            } else {
                                pixel = (pixel & 0xf0) >> 4;
                            }
                            if (pixel)
                                drawRotateSprPixel(pixel, x, y, endx - x1, y1,
                                                   endx / 2, endy, c, s);
                        }
                    }
                } else {
                    for (y1 = 0; y1 < h; y1++) {
                        if (y1 + y >= -h && y1 + y < 128 + h) {
                            for (x1 = 0; x1 < w; x1++) {
                                if (x1 + x >= -w && x1 + x < 128 + w) {
                                    pixel = *(adr + x1 + y1 * w);
                                    if ((pixel & 0xf0) > 0)
                                        drawRotateSprPixel(pixel >> 4, x, y,
                                                           ww - x1 * 2, y1, w,
                                                           h / 2, c, s);
                                    if ((pixel & 0x0f) > 0)
                                        drawRotateSprPixel(pixel & 0xf, x, y,
                                                           ww - (x1 * 2 + 1),
                                                           y1, w, h / 2, c, s);
                                }
                            }
                        }
                    }
                }
            } else {
                if (sz != 1 << fixed_res_bit) {
                    endx = ((ww * sz) >> fixed_res_bit);
                    endy = ((h * sz / 2) >> fixed_res_bit);
                    for (y1 = 0; y1 < ((h * sz) >> fixed_res_bit); y1++) {
                        y2 = ((y1 << fixed_res_bit) + 1) / sz;
                        if ((y + y1) > 128)
                            return;
                        for (x1 = 0; x1 < endx; x1++) {
                            x2 = ((x1 << fixed_res_bit) + 1) / sz;
                            pixel = *(adr + x2 / 2 + (y2 * ww) / 2);
                            if (x2 & 1) {
                                pixel = (pixel & 0x0f);
                            } else {
                                pixel = (pixel & 0xf0) >> 4;
                            }
                            if (pixel)
                                drawRotateSprPixel(pixel, x, y, x1, y1,
                                                   endx / 2, endy, c, s);
                        }
                    }
                } else {
                    for (y1 = 0; y1 < h; y1++) {
                        if (y1 + y >= -h && y1 + y < 128 + h) {
                            for (x1 = 0; x1 < w; x1++) {
                                if (x1 + x >= -w && x1 + x < 128 + w) {
                                    pixel = *(adr + x1 + y1 * w);
                                    if ((pixel & 0xf0) > 0)
                                        drawRotateSprPixel(pixel >> 4, x, y,
                                                           x1 * 2, y1, w, h / 2,
                                                           c, s);
                                    if ((pixel & 0x0f) > 0)
                                        drawRotateSprPixel(pixel & 0xf, x, y,
                                                           x1 * 2 + 1, y1, w,
                                                           h / 2, c, s);
                                }
                            }
                        }
                    }
                }
            }
        }
    } else {
        i = 0;
        pixel = sprite_table[n].flags >> 4;
        if (!sprite_table[n].angle) {
            if (SPRITE_IS_FLIP_HORIZONTAL(n)) {
                if (sz != 1 << fixed_res_bit) {
                    for (y1 = 0; y1 < ((h * sz) >> fixed_res_bit); y1++) {
                        y2 = ((y1 << fixed_res_bit) + 1) / sz;
                        if ((y + y1) > 128)
                            return;
                        for (x1 = 0; x1 < ((ww * sz) >> fixed_res_bit); x1++) {
                            x2 = ((x1 << fixed_res_bit) + 1) / sz;
                            ibit = *(adr + (x2 + y2 * ww) / 8);
                            if (ibit & (1 << (7 - ((x2 + y2 * ww) & 7))))
                                drawSprPixel(pixel, x, y, ww - x1, y1);
                        }
                    }
                } else {
                    for (y1 = 0; y1 < h; y1++) {
                        for (x1 = 0; x1 < ww; x1++) {
                            if (i % 8 == 0) {
                                ibit = *(adr);
                                adr++;
                            }
                            if (ibit & 0x80)
                                drawSprPixel(pixel, x, y, ww - x1, y1);
                            ibit = ibit << 1;
                            i++;
                        }
                    }
                }
            } else {
                if (sz != 1 << fixed_res_bit) {
                    for (y1 = 0; y1 < ((h * sz) >> fixed_res_bit); y1++) {
                        y2 = ((y1 << fixed_res_bit) + 1) / sz;
                        if ((y + y1) > 128)
                            return;
                        for (x1 = 0; x1 < ((ww * sz) >> fixed_res_bit); x1++) {
                            x2 = ((x1 << fixed_res_bit) + 1) / sz;
                            ibit = *(adr + (x2 + y2 * ww) / 8);
                            if (ibit & (1 << (7 - ((x2 + y2 * ww) & 7))))
                                drawSprPixel(pixel, x, y, x1, y1);
                        }
                    }
                } else {
                    for (y1 = 0; y1 < h; y1++) {
                        for (x1 = 0; x1 < ww; x1++) {
                            if (i % 8 == 0) {
                                ibit = *(adr);
                                adr++;
                            }
                            if (ibit & 0x80)
                                drawSprPixel(pixel, x, y, x1, y1);
                            ibit = ibit << 1;
                            i++;
                        }
                    }
                }
            }
        } else {
            c = fixed_cos(sprite_table[n].angle);
            s = fixed_sin(sprite_table[n].angle);
            if (SPRITE_IS_FLIP_HORIZONTAL(n)) {
                if (sz != 1 << fixed_res_bit) {
                    for (y1 = 0; y1 < ((h * sz) >> fixed_res_bit); y1++) {
                        y2 = ((y1 << fixed_res_bit) + 1) / sz;
                        if ((y + y1) > 128)
                            return;
                        for (x1 = 0; x1 < ((ww * sz) >> fixed_res_bit); x1++) {
                            x2 = ((x1 << fixed_res_bit) + 1) / sz;
                            ibit = *(adr + (x2 + y2 * ww) / 8);
                            if (ibit & (1 << (7 - ((x2 + y2 * ww) & 7))))
                                drawRotateSprPixel(pixel, x, y, ww - x1, y1, w,
                                                   h / 2, c, s);
                        }
                    }
                } else {
                    for (y1 = 0; y1 < h; y1++) {
                        for (x1 = 0; x1 < ww; x1++) {
                            if (i % 8 == 0) {
                                ibit = *(adr);
                                adr++;
                            }
                            if (ibit & 0x80)
                                drawRotateSprPixel(pixel, x, y, ww - x1, y1, w,
                                                   h / 2, c, s);
                            ibit = ibit << 1;
                            i++;
                        }
                    }
                }
            } else {
                if (sz != 1 << fixed_res_bit) {
                    for (y1 = 0; y1 < ((h * sz) >> fixed_res_bit); y1++) {
                        y2 = ((y1 << fixed_res_bit) + 1) / sz;
                        if ((y + y1) > 128)
                            return;
                        for (x1 = 0; x1 < ((ww * sz) >> fixed_res_bit); x1++) {
                            x2 = ((x1 << fixed_res_bit) + 1) / sz;
                            ibit = *(adr + (x2 + y2 * ww) / 8);
                            if (ibit & (1 << (7 - ((x2 + y2 * ww) & 7))))
                                drawRotateSprPixel(pixel, x, y, x1, y1, w,
                                                   h / 2, c, s);
                        }
                    }
                } else {
                    for (y1 = 0; y1 < h; y1++) {
                        for (x1 = 0; x1 < ww; x1++) {
                            if (i % 8 == 0) {
                                ibit = *(adr);
                                adr++;
                            }
                            if (ibit & 0x80)
                                drawRotateSprPixel(pixel, x, y, x1, y1, w,
                                                   h / 2, c, s);
                            ibit = ibit << 1;
                            i++;
                        }
                    }
                }
            }
        }
    }
}
void redrawSprites() {
    uint16_t i, j, ind, tempa, tempb;
    for (i = 0; i < SPRITE_COUNT; i++) {
        pix_buffer[i + SPRITE_COUNT] = i;
        pix_buffer[i] = sprite_table[i].zindex;
    }
    for (j = 0; j < SPRITE_COUNT; ++j) {
        tempa = pix_buffer[j];
        tempb = pix_buffer[j + SPRITE_COUNT];
        ind = j;
        for (i = j + 1; i < SPRITE_COUNT; ++i) {
            if (tempa > pix_buffer[i]) {
                tempa = pix_buffer[j];
                tempb = pix_buffer[j + SPRITE_COUNT];
                ind = i;
            }
        }
        pix_buffer[ind] = pix_buffer[j];
        pix_buffer[ind + SPRITE_COUNT] = pix_buffer[j + SPRITE_COUNT];
        pix_buffer[j] = tempa;
        pix_buffer[j + SPRITE_COUNT] = tempb;
    }
    for (j = 0; j < SPRITE_COUNT; j++) {
        i = pix_buffer[j + SPRITE_COUNT];
        if (sprite_table[i].lives > 0) {
            if ((sprite_table[i].x >> 2) + sprite_table[i].width < 0 ||
                (sprite_table[i].x >> 2) > 127 ||
                (sprite_table[i].y >> 2) + sprite_table[i].height < 0 ||
                (sprite_table[i].y >> 2) > 127) {
                if (sprite_table[i].onexitscreen != NULL)
                    sprite_table[i].onexitscreen(i);

            } else
                drawSpr(i, sprite_table[i].x >> 2, sprite_table[i].y >> 2);
        }
    }
}
void redrawScreen() { // todo: change the name to display. "screen" is for
                      // canvas only
    // left shift so we can use fix-point number
    // use (_ * _ratio) >> 16 for mapping
    int x_ratio = (int)((SCREEN_WIDTH << 16) / rscreenWidth);
    int y_ratio = (int)((SCREEN_HEIGHT << 16) / rscreenHeight);
    int x_canvas, hx2, y_canvas, startx;
    // Since each line in canvas might be mapped to mutiple
    // lines in display. This could reduce the compution need for redraw
    int prev_y_canvas = -1;

    for (int y_physical = 0; y_physical < rscreenHeight; y_physical++) {
        y_canvas = ((y_physical * y_ratio) >> 16);
        if (line_is_draw[y_canvas >> 5] & (1 << (y_canvas & 31))) {
            startx = displayXOffset;
            if (prev_y_canvas != y_canvas)
                for (uint16_t x_physical = 0; x_physical < rscreenWidth;
                     x_physical++) {
                    x_canvas = ((x_physical * x_ratio) >> 16);
                    hx2 = x_canvas / 2;
                    if (x_canvas & 1) {
                        if ((sprite_screen[SCREEN_ADDR(hx2, y_canvas)] & 0xf))
                            pix_buffer[x_physical] = sprtpalette[(
                                sprite_screen[SCREEN_ADDR(hx2, y_canvas)] &
                                0xf)];
                        else
                            pix_buffer[x_physical] = palette[(
                                screen[SCREEN_ADDR(hx2, y_canvas)] & 0xf)];
                    } else {
                        if ((sprite_screen[SCREEN_ADDR(hx2, y_canvas)] & 0xf0))
                            pix_buffer[x_physical] =
                                sprtpalette[(sprite_screen[SCREEN_ADDR(
                                                 hx2, y_canvas)] &
                                             0xf0) >>
                                            4];
                        else
                            pix_buffer[x_physical] =
                                palette[(screen[SCREEN_ADDR(hx2, y_canvas)] &
                                         0xf0) >>
                                        4];
                    }
                }
            prev_y_canvas = y_canvas;
            ge0_port_display_drawLine(y_physical, startx, rscreenWidth,
                                      pix_buffer);
        }
    }
    ge0_port_display_sync();
    for (uint16_t i = 0; i < 4; i++)
        line_is_draw[i] = 0;
}
