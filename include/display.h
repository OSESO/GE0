#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <stdint.h>

// todo: change this two
// this two in orig project keep the real hw size of the screen
// while the render size is configurable
#define SCREEN_REAL_WIDTH 128
#define SCREEN_REAL_HEIGHT 128

void setScreenResolution(uint16_t nw, uint16_t nh);
void redrawScreen(void);

#endif
