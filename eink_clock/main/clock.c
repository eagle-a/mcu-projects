#include "clock.h"
#include "epd_driver.h"
#include "rtc.h"
#include "font.h"
#include "config.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "CLOCK";
static uint8_t display_buffer[EPD_ARRAY];

// Set pixel in buffer (x: 0-399, y: 0-299)
static void set_pixel(uint8_t *buf, uint16_t x, uint16_t y, bool color)
{
    if (x >= EPD_WIDTH || y >= EPD_HEIGHT) return;
    
    uint32_t byte_index = (y * EPD_WIDTH + x) / 8;
    uint8_t bit_index = 7 - (x % 8);
    
    if (color) {
        buf[byte_index] |= (1 << bit_index);
    } else {
        buf[byte_index] &= ~(1 << bit_index);
    }
}

// Draw centered time string using large font (16x32 pixels)
static void draw_centered_time(uint8_t *buf, const char *str, uint16_t y)
{
    uint16_t char_width = 16;
    uint16_t char_height = 32;
    uint16_t colon_width = 8;
    uint16_t spacing = 4;
    
    uint16_t len = strlen(str);
    uint16_t digit_count = 0;
    uint16_t colon_count = 0;
    
    for (uint16_t i = 0; i < len; i++) {
        if (str[i] >= '0' && str[i] <= '9') digit_count++;
        else if (str[i] == ':') colon_count++;
    }
    
    uint16_t total_width = digit_count * char_width + colon_count * colon_width + (len - 1) * spacing;
    uint16_t x = (EPD_WIDTH - total_width) / 2;
    
    uint16_t pos = x;
    for (uint16_t i = 0; i < len; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            font_draw_large_digit(buf, pos, y, str[i] - '0');
            pos += char_width + spacing;
        } else if (str[i] == ':') {
            font_draw_large_colon(buf, pos, y);
            pos += colon_width + spacing;
        } else {
            pos += spacing;
        }
    }
}

// Draw centered date string using medium font (12x24 pixels)
static void draw_centered_date(uint8_t *buf, const char *str, uint16_t y)
{
    uint16_t char_width = 12;
    uint16_t char_height = 24;
    uint16_t colon_width = 6;
    uint16_t spacing = 2;
    
    uint16_t len = strlen(str);
    uint16_t digit_count = 0;
    uint16_t dash_count = 0;
    
    for (uint16_t i = 0; i < len; i++) {
        if (str[i] >= '0' && str[i] <= '9') digit_count++;
        else if (str[i] == '-') dash_count++;
    }
    
    uint16_t total_width = digit_count * char_width + dash_count * 8 + (len - 1) * spacing;
    uint16_t x = (EPD_WIDTH - total_width) / 2;
    
    uint16_t pos = x;
    for (uint16_t i = 0; i < len; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            font_draw_medium_digit(buf, pos, y, str[i] - '0');
            pos += char_width + spacing;
        } else if (str[i] == '-') {
            // Draw dash as a simple line
            for (uint16_t dx = 0; dx < 8; dx++) {
                set_pixel(buf, pos + dx, y + char_height / 2, true);
            }
            pos += 8 + spacing;
        } else {
            pos += spacing;
        }
    }
}

// Draw status string using small font (8x16 pixels)
static void draw_status(uint8_t *buf, uint16_t x, uint16_t y, const char *str)
{
    uint16_t char_width = 8;
    uint16_t char_height = 16;
    uint16_t spacing = 1;
    
    uint16_t pos = x;
    for (uint16_t i = 0; i < strlen(str); i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            font_draw_small_digit(buf, pos, y, str[i] - '0');
            pos += char_width + spacing;
        } else if (str[i] == ':') {
            font_draw_small_colon(buf, pos, y);
            pos += 4 + spacing;
        } else if (str[i] >= 'A' && str[i] <= 'Z') {
            // For simplicity, draw uppercase letters as small blocks
            for (uint8_t row = 0; row < char_height; row++) {
                for (uint8_t col = 0; col < char_width; col++) {
                    set_pixel(buf, pos + col, y + row, true);
                }
            }
            pos += char_width + spacing;
        } else {
            pos += spacing;
        }
    }
}

// Draw rectangle
static void draw_rectangle(uint8_t *buf, uint16_t x, uint16_t y, uint16_t width, uint16_t height, bool fill)
{
    for (uint16_t py = y; py < y + height; py++) {
        for (uint16_t px = x; px < x + width; px++) {
            bool draw = fill || (px == x || px == x + width - 1 || py == y || py == y + height - 1);
            if (draw) {
                set_pixel(buf, px, py, true);
            }
        }
    }
}

void clock_draw_face(void)
{
    ESP_LOGI(TAG, "Drawing clock face");
    
    // Clear buffer (white background)
    memset(display_buffer, 0xFF, sizeof(display_buffer));
    
    // Draw border
    draw_rectangle(display_buffer, 5, 5, EPD_WIDTH - 10, EPD_HEIGHT - 10, false);
    
    // Draw inner border
    draw_rectangle(display_buffer, 10, 10, EPD_WIDTH - 20, EPD_HEIGHT - 20, false);
    
    // Display the initial clock face
    epd_hw_init();
    epd_display_full(display_buffer);
    epd_deep_sleep();
}

void clock_update_display(void)
{
    char time_buf[16];
    char date_buf[16];
    
    app_rtc_get_time_str(time_buf, sizeof(time_buf));
    app_rtc_get_date_str(date_buf, sizeof(date_buf));
    
    ESP_LOGI(TAG, "Updating display: %s %s", time_buf, date_buf);
    
    // Clear buffer (white background)
    memset(display_buffer, 0xFF, sizeof(display_buffer));
    
    // Draw border
    draw_rectangle(display_buffer, 5, 5, EPD_WIDTH - 10, EPD_HEIGHT - 10, false);
    
    // Draw inner border
    draw_rectangle(display_buffer, 10, 10, EPD_WIDTH - 20, EPD_HEIGHT - 20, false);
    
    // Draw time (centered, large font at y=100)
    draw_centered_time(display_buffer, time_buf, 100);
    
    // Draw date (centered, medium font at y=180)
    draw_centered_date(display_buffer, date_buf, 180);
    
    // Draw WiFi status (small font at bottom)
    if (app_rtc_is_time_synced()) {
        draw_status(display_buffer, 20, 280, "WiFi:OK");
    } else {
        draw_status(display_buffer, 20, 280, "WiFi:--");
    }
    
    // Full refresh for initial display
    static bool first_update = true;
    if (first_update) {
        epd_hw_init();
        epd_display_full(display_buffer);
        first_update = false;
    } else {
        // For now, use full refresh (partial refresh needs more work)
        epd_hw_init();
        epd_display_full(display_buffer);
    }
    
    epd_deep_sleep();
}

void clock_init(void)
{
    ESP_LOGI(TAG, "Initializing clock display");
    
    // Draw initial clock face
    clock_draw_face();
    
    // Update with current time
    clock_update_display();
}
