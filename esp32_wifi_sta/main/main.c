#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "st7735.h"
#include "wifi.h"
#include "led.h"
#include "lvgl_port.h"
#include "lvgl.h"

static const char *TAG = "APP_MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 WiFi STA 启动");

    // 初始化 ST7735 屏幕
    ESP_LOGI(TAG, "开始初始化 ST7735...");
    st7735_init();
    ESP_LOGI(TAG, "ST7735 初始化完成");

    // 测试清屏 - 黑色
    st7735_clear_screen(0x0000);

    // 初始化 LED
    led_init();

    // 创建 LED 闪烁任务
    led_start_blink_task();

    // 初始化 NVS 和 WiFi
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();

    // 等待 WiFi 连接
    vTaskDelay(pdMS_TO_TICKS(3000));

    // 初始化 LVGL
    ESP_LOGI(TAG, "开始初始化 LVGL...");
    lvgl_port_init();
    ESP_LOGI(TAG, "LVGL 初始化完成");

    // 创建 LVGL 定时器任务
    xTaskCreate(lvgl_timer_task, "lvgl_timer", 8192, NULL, 6, NULL);

    // 创建简单的 LVGL 标签
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello LVGL!");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // 主任务保持运行，可以添加其他逻辑
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(TAG, "系统运行中...");
    }
}
