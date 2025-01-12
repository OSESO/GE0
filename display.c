#include "display.h"
#include "screen.h"
#include "ge0_port_interface.h"
#include <stdbool.h>

// This file includes actual logic to draw on hardware

uint16_t rscreenWidth, rscreenHeight; // Real screen size
uint16_t displayXOffset;              // Margin of the display area
uint32_t pix_buffer[SCREEN_REAL_WIDTH];

// 在GE0-YSYX中转换，ge0_port_interface接口应当统一使用RGB565颜色格式
// static uint32_t rgb565_to_rgb888(uint16_t rgb565) {
//     // See https://stackoverflow.com/a/2445096
//     uint8_t r = (rgb565 >> 11) & 0x1F;
//     uint8_t g = (rgb565 >> 5) & 0x3F;
//     uint8_t b = rgb565 & 0x1F;
//     r = (r << 3) | (r >> 2);
//     g = (g << 2) | (g >> 4);
//     b = (b << 3) | (b >> 2);
//     return (r << 16) | (g << 8) | b;
// }

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
                    // 应该在GE0-YSYX中进行转换，ge0_port_interface接口应当统一使用RGB565颜色格式
                    // pix_buffer[x_physical] =
                    //     rgb565_to_rgb888(pix_buffer[x_physical]);
                }
            prev_y_canvas = y_canvas;
            am_display_drawLine(y_physical, startx, rscreenWidth, pix_buffer);
        }
    }
    am_display_sync();
    for (uint16_t i = 0; i < 4; i++)
        line_is_draw[i] = 0;
}
