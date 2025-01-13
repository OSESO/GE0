#ifndef ge0_port_interface_H_
#define ge0_port_interface_H_

#include <stdint.h>

/**
 * @brief 填充整个屏幕为color颜色
 * 
 * @param color 填充颜色，格式：16 bit color(rgb565)
 */
void ge0_port_display_fillScreen(uint16_t color);

/**
 * @brief 
 * 
 * @param line y
 * @param start x
 * @param width 线长
 * @param colors 填充颜色缓冲，格式：16 bit color(rgb565)
 */
void ge0_port_display_drawLine(uint32_t line, uint32_t start, uint32_t width, uint32_t *colors);

/**
 * @brief 同步
 * 
 */
void ge0_port_display_sync(void);

#endif // ge0_port_interface_H_