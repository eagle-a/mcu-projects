#include <stdio.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"
#include "st7735.h"
#include "wifi.h"
#include "led.h"
#include "rtc.h"
#include "lvgl_port.h"
#include "lvgl.h"

static const char *TAG = "APP_MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 WiFi STA 启动");

    // 初始化 SPIFFS
    ESP_LOGI(TAG, "初始化 SPIFFS...");
    esp_vfs_spiffs_conf_t spiffs_conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    esp_err_t ret = esp_vfs_spiffs_register(&spiffs_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPIFFS 挂载失败: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "SPIFFS 挂载成功");
        size_t total = 0, used = 0;
        esp_spiffs_info(NULL, &total, &used);
        ESP_LOGI(TAG, "SPIFFS 总大小: %d KB, 已用: %d KB", total / 1024, used / 1024);
    }

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

    // 初始化 RTC
    ESP_LOGI(TAG, "初始化 RTC...");
    app_rtc_init();
    ESP_LOGI(TAG, "RTC 初始化完成");

    // 初始化 LVGL
    ESP_LOGI(TAG, "开始初始化 LVGL...");
    lvgl_port_init();
    ESP_LOGI(TAG, "LVGL 初始化完成");

    // 设置屏幕背景为黑色
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, 0);

    // 创建 LVGL 定时器任务
    xTaskCreate(lvgl_timer_task, "lvgl_timer", 8192, NULL, 6, NULL);

    // 创建时间标签
    lv_obj_t *time_label = lv_label_create(lv_screen_active());
    lv_label_set_text(time_label, "00:00:00");
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
    lv_obj_align(time_label, LV_ALIGN_TOP_MID, 0, 10);

    // 创建日期标签
    lv_obj_t *date_label = lv_label_create(lv_screen_active());
    lv_label_set_text(date_label, "2024-01-01");
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(date_label, lv_color_white(), 0);
    lv_obj_align(date_label, LV_ALIGN_TOP_MID, 0, 40);

    // 创建 WiFi 标签
    lv_obj_t *wifi_label = lv_label_create(lv_screen_active());
    lv_label_set_text(wifi_label, "WiFi: Connecting...");
    lv_obj_set_style_text_font(wifi_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(wifi_label, lv_color_white(), 0);
    lv_obj_align(wifi_label, LV_ALIGN_BOTTOM_MID, 0, -10);

    // 创建 FPS 标签
    lv_obj_t *fps_label = lv_label_create(lv_screen_active());
    lv_label_set_text(fps_label, "FPS: 0");
    lv_obj_set_style_text_font(fps_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(fps_label, lv_color_white(), 0);
    lv_obj_align(fps_label, LV_ALIGN_TOP_MID, 0, 60);

    // 创建动画进度条
    lv_obj_t *bar = lv_bar_create(lv_screen_active());
    lv_obj_set_size(bar, 100, 20);
    lv_obj_align(bar, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(bar, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);

    // 更新时间显示的循环
    uint32_t frame_count = 0;
    uint32_t last_fps_time = xTaskGetTickCount();
    int32_t bar_value = 0;
    int8_t bar_direction = 1;
    
    while (1) {
        char time_buf[16];
        char date_buf[16];
        app_rtc_get_time_str(time_buf, sizeof(time_buf));
        app_rtc_get_date_str(date_buf, sizeof(date_buf));
        
        ESP_LOGI(TAG, "时间更新: %s %s", time_buf, date_buf);
        
        lv_label_set_text(time_label, time_buf);
        lv_label_set_text(date_label, date_buf);
        
        if (app_rtc_is_time_synced()) {
            lv_label_set_text(wifi_label, "WiFi: Connected");
        } else {
            lv_label_set_text(wifi_label, "WiFi: No NTP");
        }
        
        // 动画进度条
        bar_value += bar_direction * 5;
        if (bar_value >= 100) {
            bar_value = 100;
            bar_direction = -1;
        } else if (bar_value <= 0) {
            bar_value = 0;
            bar_direction = 1;
        }
        lv_bar_set_value(bar, bar_value, LV_ANIM_OFF);
        
        // 强制刷新显示
        lv_obj_invalidate(time_label);
        lv_obj_invalidate(date_label);
        lv_obj_invalidate(wifi_label);
        lv_obj_invalidate(bar);
        
        // 计算 FPS
        frame_count++;
        uint32_t current_time = xTaskGetTickCount();
        if (current_time - last_fps_time >= pdMS_TO_TICKS(1000)) {
            uint32_t fps = frame_count * 1000 / (current_time - last_fps_time);
            char fps_buf[16];
            snprintf(fps_buf, sizeof(fps_buf), "FPS: %lu", fps);
            lv_label_set_text(fps_label, fps_buf);
            lv_obj_invalidate(fps_label);
            
            frame_count = 0;
            last_fps_time = current_time;
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
