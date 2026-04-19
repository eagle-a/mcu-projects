#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_D4_GPIO 12
#define LED_D5_GPIO 13

#define BLINK_DELAY_MS 500

void app_main(void)
{
    // 配置 GPIO12 和 GPIO13 为输出
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_D4_GPIO) | (1ULL << LED_D5_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    printf("LED 控制启动\n");
    printf("D4 (GPIO12) - 高电平有效\n");
    printf("D5 (GPIO13) - 高电平有效\n");

    while (1) {
        // D4 亮，D5 灭
        gpio_set_level(LED_D4_GPIO, 1);
        gpio_set_level(LED_D5_GPIO, 0);
        printf("D4 ON, D5 OFF\n");
        vTaskDelay(BLINK_DELAY_MS / portTICK_PERIOD_MS);

        // D4 灭，D5 亮
        gpio_set_level(LED_D4_GPIO, 0);
        gpio_set_level(LED_D5_GPIO, 1);
        printf("D4 OFF, D5 ON\n");
        vTaskDelay(BLINK_DELAY_MS / portTICK_PERIOD_MS);

        // 两个都亮
        gpio_set_level(LED_D4_GPIO, 1);
        gpio_set_level(LED_D5_GPIO, 1);
        printf("D4 ON, D5 ON\n");
        vTaskDelay(BLINK_DELAY_MS / portTICK_PERIOD_MS);

        // 两个都灭
        gpio_set_level(LED_D4_GPIO, 0);
        gpio_set_level(LED_D5_GPIO, 0);
        printf("D4 OFF, D5 OFF\n");
        vTaskDelay(BLINK_DELAY_MS / portTICK_PERIOD_MS);
    }
}
