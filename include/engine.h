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
typedef enum sprite_attr {
    S_X = 0,     // the x co-ordinate
    S_Y,         // the y co-ordinate
    S_SPEEDX,    // the Speed in the x direction
    S_SPEEDY,    // the Speed in the y direction
    S_WIDTH,     // the Width
    S_HEIGHT,    // the Height
    S_ANGLE,     // the Angle of the Sprite (0-360)
    S_LIVES,     // number of Lives for the Sprite
    S_COLLISION, // the ID of the Sprite that this Sprite is currently colliding
                 // with
    S_SOLID,     // whether the Sprite is Solid, or not. 1 = True, 0 = False
    S_GRAVITY,   // whether the Sprite is affected by Gravity, or not.
                 // 1 = True, 0 = False
    S_ON_COLLISION,   // the Function to execute when the Sprite collides with
                      // another Sprite.
    S_ON_EXIT_SCREEN, // the Function to execute when the Sprite moves off
                      // Screen.
    S_IS_SCROLLED,    // whether the Sprite is scrolled with the Screen, or not.
                      // 1 = True, 0 = False
    S_IS_ONEBIT, // whether the Sprite is 1 bit, or not. 1 = True, 0 = False
    S_FLIP_HORIZONTAL, // flip Sprite horizontally 1 = True, 0 = False
    S_Z_INDEX,         // Affect the Z position when scaling
    S_COLOR
} SpriteAttribute;
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

/**
   sets the value denoted by value of the type denoted by type
   for the Sprite denoted by n.
*/
void spritesetvalue(int n, enum sprite_attr type, int value);

/**
   returns the angle, in degrees, between the two Sprites denoted by n1 & n2.
   always positive
*/
int angbetweenspr(int n1, int n2);

/**
   gets the Sprite denoted by address (i.e. the name of the Sprite in its
   definition) and assigns it to the Sprite denoted by n.
   Maybe this should called setsprite instead
*/
void getsprite(int n, int address);

/**
   puts the Sprite n at the co-ordinates on the Screen denoted by x & y.
*/
void putsprite(int n, int x, int y);

/**
   Sets the sprite to n speed in the direction (0 - 360 degrees).
 */
void spritespeed(int n, int speed, int direction);

/**
   Gets the number of the sprite which is in the x & y position
*/
int getspriteinxy(int x, int y);

// *****************************
// Working with geometry
// *****************************

/**
   puts a Pixel at the co-ordinates on the Screen denoted by x & y.
*/
void putpixel(int x, int y);

/**
   returns the contents of the Pixel at the co-ordinates denoted by x & y.
*/
int getpixel(int x, int y);

/**
   draws a line from point (x, y) to point (x1, y1).
*/
void line(int x, int y, int x1, int y1);

/**
   draws a square with the top left corner at point (x, y) and the bottom right
   corner at point (x1, y1).
*/
void rect(int x, int y, int x1, int y1);

/**
   draws a filled square with the top left corner at point (x, y) and the bottom
   right corner at point (x1, y1).
*/
void fillrect(int x, int y, int x1, int y1);

/**
   draws a triangle between points (x, y), (x1, y1), and (x2, y2).
*/
void triangle(int x, int y, int x1, int y1, int x2, int y2);

/**
   draws a filled triangle between points (x, y), (x1, y1), and (x2, y2).
*/
void filltriangle(int x, int y, int x1, int y1, int x2, int y2);

/**
   draws a circle with its center at point (x, y) and radius r.
*/
void circle(int x, int y, int r);

/**
   draws a filled circle with its center at point (x, y) and radius r.
*/
void fillcircle(int x, int y, int r);

// *****************************
// Working with strings
// *****************************

/**
   prints a character to the Screen at the current x & y text co-ordinates.
*/
char putchar(char c);

/**
   prints a 1-D char array to the Screen at the current x & y co-ordinates.
*/
int puts(char *message);

/**
   prints an integer in decimal format.
*/
int putn(int number);

/**
   prints a line with a specified formatting. Supported %d (number int) %f
   (number fixed) %s (string) %c (character).
*/
// int printf(char *formatString, arg - list...);

/**
   gets a character from the keyboard. The program is interrupted and waits for
   the character to be entered.
*/
int getchar();

/**
   Load the font picture and initialize the characters from first to end. The
   characters should follow the order according to ASCII.
*/
void loadfont(char *font, char first, char end);

/**
   Sets the size of the font previously loaded.
*/
void setfontsize(int picture_width, int picture_height, int character_width,
                 int character_height);

/**
   Draws the text on the screen at coordinates x and y.
*/
void drawstring(char *text, int x, int y);

/**
   Draws the character on the screen at coordinates x and y.
*/
void drawchar(char character, int x, int y);

#endif
