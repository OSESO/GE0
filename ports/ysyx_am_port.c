#include "ge0_port_interface.h"

#include "am.h" // IWYU pragma: keep.
#include "amdev.h"
#include "klib-macros.h"
#include "klib.h" // IWYU pragma: keep

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

void ge0_port_display_fillScreen(uint16_t color) {
    //  todo: read those two only one time
    int height = io_read(AM_GPU_CONFIG).height;
    int width = io_read(AM_GPU_CONFIG).width;
    /* uint32_t empty_buffer[height * width]; */ // 爆栈了！
    uint32_t *empty_buffer =
        (uint32_t *)malloc(height * width * sizeof(uint32_t));
    uint32_t color_888 = rgb565_to_rgb888(color);
    for (int i = 0; i < height * width; i++) {
        /* printf("Current i is %d\n",i); */
        empty_buffer[i] = color_888;
    }
    io_write(AM_GPU_FBDRAW, 0, 0, empty_buffer, width, height, false);
    io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, true);
    free(empty_buffer);
}

void ge0_port_display_drawLine(uint32_t line, uint32_t start, uint32_t width,
                               uint32_t *colors) {
    for (uint32_t i = 0; i < width; ++i)
        colors[i] = rgb565_to_rgb888(colors[i]);
    io_write(AM_GPU_FBDRAW, start, line, colors, width, 1, 0);
}

void ge0_port_display_sync(void) {
    io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, 1);
}

int ge0_port_get_key(void) {
    AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
    if (ev.keycode == AM_KEY_NONE || ev.keydown == false)
        return 0;
    int ret = 0;
    switch (ev.keycode) {
    case AM_KEY_UP:
        ret = 1;
        break;
    case AM_KEY_LEFT:
        ret = 4;
        break;
    case AM_KEY_DOWN:
        ret = 2;
        break;
    case AM_KEY_RIGHT:
        ret = 8;
        break;
    case AM_KEY_Z: // key_a
        ret = 16;
        break;
    case AM_KEY_X: // key_b
        ret = 32;
        break;
    case AM_KEY_C: // key_select
        ret = 64;
        break;
    case AM_KEY_V: // key_start
        ret = 128;
        break;
    }
    return ret;
}

int ge0_port_itoa(int num, char *str, int radix) {
    return klib__itoa(num, str, radix);
}
#ifdef MEMORY_BY_GE0

void *ge0_port_malloc(int size) { return malloc(size); }
void ge0_port_free(void *array) { free(array); }
void ge0_port_memcpy(int *array1, int *array2, int size) {
    memcpy(array1, array2, size);
}

#endif // MEMORY_BY_GE0
