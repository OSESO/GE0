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
void ge0_port_display_drawLine(uint32_t line, uint32_t start, uint32_t width,
                               uint32_t *colors);

/**
 * @brief 同步
 *
 */
void ge0_port_display_sync(void);

/**
 * @brief 获得一个按键
 *
 * @retval  0 无按键
 * @retval    按键码
 */
int ge0_port_get_key(void);

/**
 * @brier 获得一个随机数
 *
 * @param max 随机数的最大值，不含
 * @retval 随机数
 */
int ge0_port_random_max(int max);

/**
 * @brier 获得一个随机数
 *
 * @param min 随机数的最小值，含
 * @param max 随机数的最大值，不含
 * @retval 随机数
 */
int ge0_port_random_min_max(int min, int max);

#endif // ge0_port_interface_H_
