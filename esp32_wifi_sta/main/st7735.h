#ifndef ST7735_H
#define ST7735_H

#include <stdint.h>
#include "driver/spi_master.h"

#define TFT_WIDTH  128
#define TFT_HEIGHT 160
#define TFT_XOFFSET  2  // 列地址偏移，修复边缘花屏
#define TFT_YOFFSET  1  // 行地址偏移

#define TFT_MOSI  3
#define TFT_SCLK  2
#define TFT_CS    7
#define TFT_DC    6
#define TFT_RST   10

#define SPI_HOST  SPI2_HOST
#define SPI_FREQ  27000000

// 初始化 SPI 和 ST7735
void st7735_init(void);

// 清屏（填充颜色）
void st7735_clear_screen(uint16_t color);

// 设置地址窗口（供 LVGL 使用）
void st7735_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

// 获取 SPI 句柄（供 LVGL 使用）
spi_device_handle_t st7735_get_spi_handle(void);

#endif // ST7735_H
