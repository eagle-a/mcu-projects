#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "led.h"

static const char *TAG = "LED";

static void led_blink_task(void *arg)
{
    ESP_LOGI(TAG, "LED 闪烁任务启动");
    while (1) {
        // D4 亮，D5 灭
        gpio_set_level(LED_D4_GPIO, 1);
        gpio_set_level(LED_D5_GPIO, 0);
        vTaskDelay(BLINK_DELAY_MS / portTICK_PERIOD_MS);

        // D4 灭，D5 亮
        gpio_set_level(LED_D4_GPIO, 0);
        gpio_set_level(LED_D5_GPIO, 1);
        vTaskDelay(BLINK_DELAY_MS / portTICK_PERIOD_MS);

        // 两个都亮
        gpio_set_level(LED_D4_GPIO, 1);
        gpio_set_level(LED_D5_GPIO, 1);
        vTaskDelay(BLINK_DELAY_MS / portTICK_PERIOD_MS);

        // 两个都灭
        gpio_set_level(LED_D4_GPIO, 0);
        gpio_set_level(LED_D5_GPIO, 0);
        vTaskDelay(BLINK_DELAY_MS / portTICK_PERIOD_MS);
    }
}

void led_init(void)
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

    ESP_LOGI(TAG, "LED 控制启动");
    ESP_LOGI(TAG, "D4 (GPIO12) - 高电平有效");
    ESP_LOGI(TAG, "D5 (GPIO13) - 高电平有效");
}

void led_start_blink_task(void)
{
    xTaskCreate(led_blink_task, "led_blink_task", 2048, NULL, 5, NULL);
    ESP_LOGI(TAG, "LED 任务已创建");
}
