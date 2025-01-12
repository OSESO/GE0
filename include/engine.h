#ifndef __ENGINE_H__
#define __ENGINE_H__

// The API from this file should be consistent with
// https://github.com/corax89/game_engine_for_esp8266_with_compiler/wiki/Standard-language-functions

// Fixed是引擎自定义的一种定点数结构
// int = fixed * (1<<8)
// fixed = int / (1<<8)
// todo:  但把图片修改为1倍大小时，代码用的是setimagesize（1）
// 这个转换到底是什么时候发生的不太清楚
// 先搁置一下
typedef int fixed;
enum sprite_attr {
    S_X = 0,
    S_Y,
    S_SPEEDX,
    S_SPEEDY,
    S_WIDTH,
    S_HEIGHT,
    S_IS_ONEBIT,
    S_ANGLE,
    S_LIVES,
    S_COLLISION,
    S_SOLID,
    S_GRAVITY,
    S_ON_COLLISION,
    S_ON_EXIT_SCREEN,
    S_IS_SCROLLED,
    S_FLIP_HORIZONTAL
};
// *****************************
// Working with the screen
// *****************************
/**
    sets the size of the Image using a multiplier indicating
    the size.Affects only the functions putimage, putimage, putimagerle

*/
void setimagesize(fixed size);

/**
    sets the area in which the drawing and scrolling takes place.
    Does not affect sprites.
*/
void setclip(int x, int y, int width, int height);

/**
    moves the Cursor to the Text screen co-ordinates denoted by x & y.
*/
void gotoxy(int x, int y);

/**
    scrolls the Screen in the direction indicated by dir.
    2 = x – 1 (i.e. Left) | 1 = y – 1 (i.e. Up)
    0 = x + 1 (i.e. Right) | any other number = y + 1  (i.e. Down)
*/
void scroll(int dir);

/**
   delays the redrawing of the Screen for a small period of time
*/
void delayredraw(void);

/**
   Clears the entire screen with the color setbgcolor. Does not affect sprites.
*/
void clearscreen(void);

/**
    Changes the framerate in the range of 1-40 frames per second.
*/
void setframerate(int fps);

// *****************************
// Image work
// *****************************
/**
   puts an image on the Screen. The image is denoted by address.
   The Screen co-ordinates by x & y, and the image width & height by w & h.
*/
void putimage(char *image, int x, int y, int w, int h);

/**
   puts a 1bit image on the Screen. The image is denoted by address. The Screen
   co-ordinates by x & y, and the image width & height by w & h.
*/
void putimage1bit(char *image, int x, int y, int w, int h);

/**
    puts an RLE image on the Screen. The image is denoted by address. The Screen
    co-ordinates by x & y, and the image width & height by w & h.
*/
void putimagerle(char *image, int x, int y, int w, int h);

// *****************************
// Work with palette and colors.
// *****************************
/**
   changes the Palette colour denoted by n
   to the colour denoted by r5g6b5 (as an RGB value).
*/
void setpalette(int n, int r5g6b5);

/**
   sets the current Pen colour to the colour from
   the Palette denoted by col (i.e. a value between 0 & 15).
*/
void setcolor(int color);

/**
   sets the current Background colour to the colour from
   the Palette denoted by col (i.e. a value between 0 & 15).
*/
void setbgcolor(int color);

// *****************************
// Working with sprites
// *****************************
/**
   returns the value for the Sprite specified in n, according to the type.
*/
int spritegetvalue(int n, enum sprite_attr type);

#endif
