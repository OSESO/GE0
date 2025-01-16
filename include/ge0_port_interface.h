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

/**
 * @brier 持久化存储一个数据
 *
 * @param name 数据的名称
 * @param data 指向存储地址
 * @param data count size of data in byte
 * @retval the data saved. If not space is avliable, return any val smaller than
 * count
 */
int ge0_port_savedata(char *name, void *data, int count);

/**
 * @brier 从持久化存储中读取数据
 *
 * @param name 数据的名称
 * @param[out] data 被填充的数据地址
 * @retval the data read.
 */
int ge0_port_loaddata(char *name, void *data);

/**
 * @brief Stop the generation of sound triggered by ge0_port_tone
 */
void ge0_port_noTone(void);

/**
 * @brief Generates a  wave of the specified frequency with duration
 */
void ge0_port_tone(int freq, short duration);

/**
 * @brief Allocates memory by returning a pointer or 0 if no memory was
 * allocated.
 * @param size The size of the memory to allocate.
 * @retval A pointer to the allocated memory or 0 if allocation failed.
 */
void *ge0_port_malloc(int size);

/**
 * @brief Fill a block of memory with a particular value
 * @param s the memory to fill
 * @param c fill byte
 * @param n number of bytes to fill
 */
void *ge0_port_memset(void *s, int c, unsigned int n);

/**
 * @brief Releases previously allocated memory.
 * @param array A pointer to the memory to be freed.
 */
void ge0_port_free(void *array);

/**
 * @brief Copies the contents of one array to another.
 * @param array1 The destination array.
 * @param array2 The source array.
 * @param size The number of elements to copy.
 */
void ge0_port_memcpy(int *array1, int *array2, int size);

/**
 * @brief Returns the number of milliseconds passed since boot
 *
 * @retval Number of milliseconds passed
 */

uint32_t ge0_port_millis(void);

#endif // ge0_port_interface_H_
