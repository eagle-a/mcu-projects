#ifndef LED_H
#define LED_H

#include <stdint.h>

#define LED_D4_GPIO 12
#define LED_D5_GPIO 13
#define BLINK_DELAY_MS 500

// 初始化 LED GPIO
void led_init(void);

// 创建 LED 闪烁任务
void led_start_blink_task(void);

#endif // LED_H
