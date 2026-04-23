#include "epd_driver.h"
#include "config.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "EPD_DRIVER";

static spi_device_handle_t spi_handle = NULL;

// SPI transaction helper
static void epd_spi_transfer(const uint8_t *data, size_t len, bool is_cmd)
{
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data,
        .rx_buffer = NULL,
        .flags = 0,
    };
    spi_device_polling_transmit(spi_handle, &t);
}

// Write command
static void epd_write_cmd(uint8_t cmd)
{
    gpio_set_level(EPD_CS_PIN, 0);   // CS low
    gpio_set_level(EPD_DC_PIN, 0);   // Command mode
    epd_spi_transfer(&cmd, 1, true);
    gpio_set_level(EPD_CS_PIN, 1);   // CS high
}

// Write data
static void epd_write_data(uint8_t data)
{
    gpio_set_level(EPD_CS_PIN, 0);   // CS low
    gpio_set_level(EPD_DC_PIN, 1);   // Data mode
    epd_spi_transfer(&data, 1, false);
    gpio_set_level(EPD_CS_PIN, 1);   // CS high
}

// Wait for display to be ready
// Note: Z96 panel busy pin is HIGH when busy
static void epd_wait_busy(void)
{
    ESP_LOGI(TAG, "Waiting for display ready...");
    uint32_t timeout = 0;
    while (gpio_get_level(EPD_BUSY_PIN) == 1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        timeout += 10;
        if (timeout > 20000) {  // 20 second timeout
            ESP_LOGW(TAG, "Busy timeout, forcing continue");
            break;
        }
    }
    ESP_LOGI(TAG, "Display ready (waited %d ms)", timeout);
}

// Initialize SPI
static void epd_spi_init(void)
{
    ESP_LOGI(TAG, "Initializing SPI for e-ink display");
    
    spi_bus_config_t buscfg = {
        .miso_io_num = -1,  // No MISO for this display
        .mosi_io_num = EPD_SPI_MOSI_PIN,
        .sclk_io_num = EPD_SPI_SCK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = EPD_ARRAY + 8,
    };
    
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = EPD_SPI_FREQ,
        .mode = 0,  // SPI_MODE0
        .spics_io_num = -1,  // Manual CS control (like reference code)
        .queue_size = 1,
        .flags = SPI_DEVICE_NO_DUMMY,
    };
    
    ESP_ERROR_CHECK(spi_bus_initialize(EPD_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(EPD_SPI_HOST, &devcfg, &spi_handle));
    
    ESP_LOGI(TAG, "SPI initialized");
}

// Initialize GPIO pins
static void epd_gpio_init(void)
{
    ESP_LOGI(TAG, "Initializing GPIO pins");
    
    // Configure control pins (RST, DC, CS - all manual control)
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << EPD_RST_PIN) | (1ULL << EPD_DC_PIN) | (1ULL << EPD_CS_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    
    // Configure BUSY pin as input with pull-up
    io_conf.pin_bit_mask = (1ULL << EPD_BUSY_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
    
    // Set initial states (CS high = inactive)
    gpio_set_level(EPD_CS_PIN, 1);
    gpio_set_level(EPD_RST_PIN, 1);
    gpio_set_level(EPD_DC_PIN, 1);
    
    ESP_LOGI(TAG, "GPIO initialized: BUSY=%d, RST=%d, DC=%d, CS=%d", 
             EPD_BUSY_PIN, EPD_RST_PIN, EPD_DC_PIN, EPD_CS_PIN);
}

void epd_init(void)
{
    ESP_LOGI(TAG, "Initializing e-ink display driver");
    
    epd_gpio_init();
    epd_spi_init();
    
    ESP_LOGI(TAG, "E-ink display driver initialized");
}

static void epd_init_display_sequence(void)
{
    // T81 (GDEY042T81) initialization sequence
    epd_wait_busy();

    // Software reset
    epd_write_cmd(0x12);
    epd_wait_busy();

    // Driver output control: (HEIGHT-1)%256, (HEIGHT-1)/256, 0x01 (mirror to fix orientation)
    epd_write_cmd(0x01);
    epd_write_data((EPD_HEIGHT - 1) % 256);  // 0x2B for 300 height
    epd_write_data((EPD_HEIGHT - 1) / 256);  // 0x01 for 300 height
    epd_write_data(0x01);  // Mirror to fix orientation

    // Display update control: 0x40, 0x00 (T81 specific)
    epd_write_cmd(0x21);
    epd_write_data(0x40);
    epd_write_data(0x00);

    // Border waveform: 0x05 (T81 specific)
    epd_write_cmd(0x3C);
    epd_write_data(0x05);

    // Data entry mode: 0x01 (Y decrement, X decrement - mirror mode)
    epd_write_cmd(0x11);
    epd_write_data(0x01);

    // RAM X address: 0x00 to WIDTH/8-1 (49 for 400 pixels)
    epd_write_cmd(0x44);
    epd_write_data(0x00);
    epd_write_data((EPD_WIDTH / 8) - 1);  // 49 = 0x31

    // RAM Y address: (HEIGHT-1) to 0x0000
    epd_write_cmd(0x45);
    epd_write_data((EPD_HEIGHT - 1) % 256);  // 0x2B
    epd_write_data((EPD_HEIGHT - 1) / 256);  // 0x01
    epd_write_data(0x00);
    epd_write_data(0x00);

    // RAM X counter: 0x00
    epd_write_cmd(0x4E);
    epd_write_data(0x00);

    // RAM Y counter: (HEIGHT-1)
    epd_write_cmd(0x4F);
    epd_write_data((EPD_HEIGHT - 1) % 256);  // 0x2B
    epd_write_data((EPD_HEIGHT - 1) / 256);  // 0x01
    
    epd_wait_busy();
}

void epd_hw_init(void)
{
    ESP_LOGI(TAG, "Full screen refresh initialization (T81)");
    
    // Reset sequence - T81 requires at least 10ms delay
    gpio_set_level(EPD_RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(EPD_RST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    epd_init_display_sequence();
}

void epd_hw_init_fast(void)
{
    ESP_LOGI(TAG, "Fast refresh initialization (T81)");
    // T81 fast init: minimal sequence
    gpio_set_level(EPD_RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(EPD_RST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    epd_wait_busy();
    epd_write_cmd(0x12);  // SWRESET
    epd_wait_busy();
    
    // Minimal init for fast update
    epd_write_cmd(0x21);
    epd_write_data(0x40);
    epd_write_data(0x00);
    epd_write_cmd(0x3C);
    epd_write_data(0x05);

    // Data entry mode (must match full init)
    epd_write_cmd(0x11);
    epd_write_data(0x01);

    // RAM X address
    epd_write_cmd(0x44);
    epd_write_data(0x00);
    epd_write_data((EPD_WIDTH / 8) - 1);

    // RAM Y address
    epd_write_cmd(0x45);
    epd_write_data((EPD_HEIGHT - 1) % 256);
    epd_write_data((EPD_HEIGHT - 1) / 256);
    epd_write_data(0x00);
    epd_write_data(0x00);

    // RAM X counter
    epd_write_cmd(0x4E);
    epd_write_data(0x00);

    // RAM Y counter
    epd_write_cmd(0x4F);
    epd_write_data((EPD_HEIGHT - 1) % 256);
    epd_write_data((EPD_HEIGHT - 1) / 256);

    epd_wait_busy();
}

static void epd_update_full(void)
{
    epd_write_cmd(0x22);
    epd_write_data(0xF7);  // T81 (GDEY042T81) full update
    epd_write_cmd(0x20);
    epd_wait_busy();
}

static void epd_update_fast(void)
{
    epd_write_cmd(0x22);
    epd_write_data(0xC7);
    epd_write_cmd(0x20);
    epd_wait_busy();
}

static void epd_update_partial(void)
{
    epd_write_cmd(0x22);
    epd_write_data(0xFF);
    epd_write_cmd(0x20);
    epd_wait_busy();
}

void epd_display_full(const uint8_t *datas)
{
    ESP_LOGI(TAG, "Displaying full screen image");
    
    // Write to RAM for black/white (0x24 command)
    epd_write_cmd(0x24);
    for (uint32_t i = 0; i < EPD_ARRAY; i++) {
        epd_write_data(datas[i]);
    }
    
    // Write to RAM for red/color (0x26 command)
    // T81 (IL0398): write inverted red buffer data
    // For black/white only display, red buffer is 0x00, so ~0x00 = 0xFF (no red)
    epd_write_cmd(0x26);
    for (uint32_t i = 0; i < EPD_ARRAY; i++) {
        epd_write_data(0xFF);  // No red pixels
    }
    
    epd_update_full();
}

void epd_clear_white(void)
{
    ESP_LOGI(TAG, "Clearing screen to white");
    
    static uint8_t white_buf[EPD_ARRAY];
    memset(white_buf, 0xFF, sizeof(white_buf));
    
    epd_display_full(white_buf);
}

void epd_clear_black(void)
{
    ESP_LOGI(TAG, "Clearing screen to black");
    
    static uint8_t black_buf[EPD_ARRAY];
    memset(black_buf, 0x00, sizeof(black_buf));
    
    epd_display_full(black_buf);
}

void epd_set_base_map(const uint8_t *datas)
{
    ESP_LOGI(TAG, "Setting base map for partial refresh");
    
    epd_write_cmd(0x24);
    for (uint32_t i = 0; i < EPD_ARRAY; i++) {
        epd_write_data(datas[i]);
    }
}

void epd_display_partial(uint16_t x_start, uint16_t y_start, 
                         const uint8_t *datas, uint16_t part_column, uint16_t part_line)
{
    ESP_LOGI(TAG, "Partial refresh at (%d, %d), size %dx%d", 
             x_start, y_start, part_column, part_line);
    
    // Set RAM X address
    epd_write_cmd(0x44);
    epd_write_data(x_start / 8);
    epd_write_data((x_start + part_column / 8) - 1);
    
    // Set RAM Y address
    epd_write_cmd(0x45);
    epd_write_data(y_start % 256);
    epd_write_data(y_start / 256);
    epd_write_data((y_start + part_line - 1) % 256);
    epd_write_data((y_start + part_line - 1) / 256);
    
    // Set RAM X address counter
    epd_write_cmd(0x4E);
    epd_write_data(x_start / 8);
    
    // Set RAM Y address counter
    epd_write_cmd(0x4F);
    epd_write_data(y_start % 256);
    epd_write_data(y_start / 256);
    
    // Write data
    epd_write_cmd(0x24);
    uint32_t data_size = (part_column / 8) * part_line;
    for (uint32_t i = 0; i < data_size; i++) {
        epd_write_data(datas[i]);
    }
    
    epd_update_partial();
}

void epd_deep_sleep(void)
{
    ESP_LOGI(TAG, "Entering deep sleep");
    epd_write_cmd(0x10);
    epd_write_data(0x01);  // T81 (IL0398) uses 0x01
    vTaskDelay(pdMS_TO_TICKS(100));
}

bool epd_is_busy(void)
{
    return gpio_get_level(EPD_BUSY_PIN) == 1;
}
