#include "display.h"
#include "amdev.h"
#include "klib-macros.h"
#include "klib.h"
#include "screen.h"
#include <stdbool.h>

// This file includes actual logic to draw on hardware

uint16_t rscreenWidth, rscreenHeight; // Real screen size
uint16_t displayXOffset;              // Margin of the display area
uint32_t pix_buffer[SCREEN_REAL_WIDTH];

static uint32_t rgb565_to_rgb888(uint16_t rgb565) {
    // See https://stackoverflow.com/a/2445096
    uint8_t r = (rgb565 >> 11) & 0x1F;
    uint8_t g = (rgb565 >> 5) & 0x3F;
    uint8_t b = rgb565 & 0x1F;
    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);
    return (r << 16) | (g << 8) | b;
}
void am_display_fillScreen(uint16_t color) { // engine uses 16 bit color(rgb565)
    //  todo: read those two only one time
    int height = io_read(AM_GPU_CONFIG).height;
    int width = io_read(AM_GPU_CONFIG).width;
    /* uint32_t empty_buffer[height * width]; */ // 爆栈了！
    uint32_t *empty_buffer = malloc(height * width * sizeof(uint32_t));
    uint32_t color_888 = rgb565_to_rgb888(color);
    for (int i = 0; i < height * width; i++) {
        /* printf("Current i is %d\n",i); */
        empty_buffer[i] = color_888;
    }
    io_write(AM_GPU_FBDRAW, 0, 0, empty_buffer, width, height, false);
    io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, true);
    free(empty_buffer);
}
void am_display_drawLine(uint32_t line, uint32_t start, uint32_t width,
                         uint32_t *buf) {
    io_write(AM_GPU_FBDRAW, start, line, buf, width, 1, 0);
}
void am_display_sync() { io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, 1); }

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
    am_display_fillScreen(0x0000);
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
                    pix_buffer[x_physical] =
                        rgb565_to_rgb888(pix_buffer[x_physical]);
                }
            prev_y_canvas = y_canvas;
            am_display_drawLine(y_physical, startx, rscreenWidth, pix_buffer);
        }
    }
    // todo remove this
    io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, true);
    for (uint16_t i = 0; i < 4; i++)
        line_is_draw[i] = 0;
}
