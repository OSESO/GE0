#include "ge0_port_interface.h"
#include "lcd.h"

void ge0_port_display_fillScreen(uint16_t color){
    lcd_fill(0, 0, LCD_W, LCD_H, color);
}

void ge0_port_display_drawLine(uint32_t line, uint32_t start, uint32_t width, uint32_t *colors){
    for(uint32_t i=0;i<width;++i){
        lcd_draw_point(start+i, line, ((uint16_t*)colors)[i]);
    }
}

void ge0_port_display_sync(void){
    return;
}