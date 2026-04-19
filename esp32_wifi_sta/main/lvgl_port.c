#include "lvgl_port.h"
#include "st7735.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

static const char *TAG = "LVGL_PORT";

static lv_display_t *disp = NULL;

// 刷新显示回调函数
static void flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    st7735_set_addr_window(area->x1, area->y1, area->x2, area->y2);
    
    spi_device_handle_t spi_handle = st7735_get_spi_handle();
    
    // 发送像素数据 - 使用 DMA 传输
    gpio_set_level(TFT_DC, 1);
    
    uint32_t px_count = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);
    uint32_t px_total = px_count;
    uint32_t px_offset = 0;
    const uint32_t max_px_per_transfer = 1024; // 每次最多传输 1024 个像素
    
    while (px_offset < px_total) {
        uint32_t px_to_send = px_total - px_offset;
        if (px_to_send > max_px_per_transfer) {
            px_to_send = max_px_per_transfer;
        }
        
        spi_transaction_t t = {
            .length = px_to_send * 16,
            .tx_buffer = px_map + px_offset * 2,
            .flags = 0,
        };
        spi_device_polling_transmit(spi_handle, &t);
        
        px_offset += px_to_send;
    }
    
    lv_display_flush_ready(display);
}

void lvgl_port_init(void)
{
    ESP_LOGI(TAG, "初始化 LVGL 显示驱动");
    
    // 初始化 LVGL
    lv_init();
    
    // 创建显示驱动
    disp = lv_display_create(TFT_WIDTH, TFT_HEIGHT);
    lv_display_set_flush_cb(disp, flush_cb);
    
    // 设置显示缓冲区
    static lv_color_t buf[TFT_WIDTH * 20];  // 20 行缓冲
    lv_display_set_buffers(disp, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
    
    ESP_LOGI(TAG, "LVGL 显示驱动初始化完成");
}

void lvgl_timer_task(void *arg)
{
    ESP_LOGI(TAG, "LVGL 定时器任务启动");
    
    while (1) {
        lv_tick_inc(10);
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
