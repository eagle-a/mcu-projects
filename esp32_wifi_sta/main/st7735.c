#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "st7735.h"

static const char *TAG = "ST7735";
static spi_device_handle_t spi_handle = NULL;

static void st7735_write_cmd(uint8_t cmd)
{
    gpio_set_level(TFT_DC, 0);
    spi_transaction_t t = {
        .length = 8,
        .tx_data[0] = cmd,
        .flags = SPI_TRANS_USE_TXDATA,
    };
    spi_device_polling_transmit(spi_handle, &t);
}

static void st7735_write_data(uint8_t data)
{
    gpio_set_level(TFT_DC, 1);
    spi_transaction_t t = {
        .length = 8,
        .tx_data[0] = data,
        .flags = SPI_TRANS_USE_TXDATA,
    };
    spi_device_polling_transmit(spi_handle, &t);
}

void st7735_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    st7735_write_cmd(0x2A); // Column address set
    st7735_write_data((x0 + TFT_XOFFSET) >> 8);
    st7735_write_data((x0 + TFT_XOFFSET) & 0xFF);
    st7735_write_data((x1 + TFT_XOFFSET) >> 8);
    st7735_write_data((x1 + TFT_XOFFSET) & 0xFF);

    st7735_write_cmd(0x2B); // Row address set
    st7735_write_data((y0 + TFT_YOFFSET) >> 8);
    st7735_write_data((y0 + TFT_YOFFSET) & 0xFF);
    st7735_write_data((y1 + TFT_YOFFSET) >> 8);
    st7735_write_data((y1 + TFT_YOFFSET) & 0xFF);

    st7735_write_cmd(0x2C); // Memory write
}

static void st7735_fill_color(uint16_t color)
{
    uint8_t color_buf[2] = {color >> 8, color & 0xFF};
    uint16_t total_pixels = TFT_WIDTH * TFT_HEIGHT;

    st7735_set_addr_window(0, 0, TFT_WIDTH - 1, TFT_HEIGHT - 1);

    for (uint16_t i = 0; i < total_pixels; i++) {
        gpio_set_level(TFT_DC, 1);
        spi_transaction_t t = {
            .length = 16,
            .tx_buffer = color_buf,
            .flags = 0,
        };
        spi_device_polling_transmit(spi_handle, &t);
    }
}

void st7735_init(void)
{
    ESP_LOGI(TAG, "初始化 ST7735...");

    // 初始化 SPI 总线
    spi_bus_config_t buscfg = {
        .mosi_io_num = TFT_MOSI,
        .sclk_io_num = TFT_SCLK,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = SPI_FREQ,
        .mode = 0,
        .spics_io_num = TFT_CS,
        .queue_size = 7,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI_HOST, &devcfg, &spi_handle));
    ESP_LOGI(TAG, "SPI 初始化完成");

    // 配置控制引脚
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << TFT_DC) | (1ULL << TFT_RST),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // 复位屏幕
    gpio_set_level(TFT_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(TFT_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    // ST7735 GREENTAB2 完整初始化序列
    st7735_write_cmd(0x01); // Software reset
    vTaskDelay(pdMS_TO_TICKS(120));

    st7735_write_cmd(0x11); // Sleep out
    vTaskDelay(pdMS_TO_TICKS(120));

    st7735_write_cmd(0xB1); // Frame rate control
    st7735_write_data(0x01);
    st7735_write_data(0x2C);
    st7735_write_data(0x2D);

    st7735_write_cmd(0xB2); // Frame rate control
    st7735_write_data(0x01);
    st7735_write_data(0x2C);
    st7735_write_data(0x2D);

    st7735_write_cmd(0xB3); // Frame rate control
    st7735_write_data(0x01);
    st7735_write_data(0x2C);
    st7735_write_data(0x2D);

    st7735_write_cmd(0xB4); // Display inversion
    st7735_write_data(0x07);

    st7735_write_cmd(0xC0); // Power control
    st7735_write_data(0xA2);
    st7735_write_data(0x02);
    st7735_write_data(0x84);

    st7735_write_cmd(0xC1); // Power control
    st7735_write_data(0xC5);

    st7735_write_cmd(0xC2); // Power control
    st7735_write_data(0x0A);
    st7735_write_data(0x00);

    st7735_write_cmd(0xC3); // Power control
    st7735_write_data(0x8A);
    st7735_write_data(0x2A);

    st7735_write_cmd(0xC4); // Power control
    st7735_write_data(0x8A);
    st7735_write_data(0xEE);

    st7735_write_cmd(0xC5); // VCOM control
    st7735_write_data(0x0E);

    st7735_write_cmd(0x36); // Memory data access control
    st7735_write_data(0xC8); // RGB order

    st7735_write_cmd(0x3A); // Set pixel format
    st7735_write_data(0x05); // 16-bit color (RGB565)

    st7735_write_cmd(0x2A); // Column address set
    st7735_write_data(0x00);
    st7735_write_data(0x00);
    st7735_write_data(0x00);
    st7735_write_data(0x7F); // 127

    st7735_write_cmd(0x2B); // Row address set
    st7735_write_data(0x00);
    st7735_write_data(0x00);
    st7735_write_data(0x00);
    st7735_write_data(0x9F); // 159

    st7735_write_cmd(0x29); // Display on

    ESP_LOGI(TAG, "ST7735 初始化完成");
}

void st7735_clear_screen(uint16_t color)
{
    ESP_LOGI(TAG, "清屏: 颜色 0x%04X", color);
    st7735_fill_color(color);
}

spi_device_handle_t st7735_get_spi_handle(void)
{
    return spi_handle;
}
