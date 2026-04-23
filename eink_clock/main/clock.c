#include "clock.h"
#include "epd_driver.h"
#include "rtc.h"
#include "font.h"
#include "config.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

static const char *TAG = "CLOCK";
static uint8_t display_buffer[EPD_ARRAY];

// Set pixel in buffer (x: 0-399, y: 0-299)
static void set_pixel(uint8_t *buf, uint16_t x, uint16_t y, bool color)
{
    if (x >= EPD_WIDTH || y >= EPD_HEIGHT) return;
    
    uint32_t byte_index = (y * EPD_WIDTH + x) / 8;
    uint8_t bit_index = 7 - (x % 8);
    
    if (color) {
        buf[byte_index] &= ~(1 << bit_index);
    } else {
        buf[byte_index] |= (1 << bit_index);
    }
}

// Draw centered time string using large font (16x32 pixels)
static void draw_centered_time(uint8_t *buf, const char *str, uint16_t y)
{
    uint16_t char_width = 16;
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

static void draw_hline(uint8_t *buf, uint16_t x1, uint16_t x2, uint16_t y)
{
    for (uint16_t x = x1; x <= x2 && x < EPD_WIDTH; x++) {
        set_pixel(buf, x, y, true);
    }
}

static void draw_vline(uint8_t *buf, uint16_t x, uint16_t y1, uint16_t y2)
{
    for (uint16_t y = y1; y <= y2 && y < EPD_HEIGHT; y++) {
        set_pixel(buf, x, y, true);
    }
}

static int parse_date(const char *s, int *y, int *m, int *d)
{
    if (!s || sscanf(s, "%d-%d-%d", y, m, d) != 3) {
        return -1;
    }
    return 0;
}

static int calc_weekday(int year, int month, int day)
{
    struct tm tm_value = {
        .tm_year = year - 1900,
        .tm_mon = month - 1,
        .tm_mday = day,
        .tm_hour = 12,
    };
    if (mktime(&tm_value) == (time_t)-1) {
        return -1;
    }
    return tm_value.tm_wday;
}

static void extract_date_only(const char *src, char *dst, size_t dst_size)
{
    size_t n = 0;
    if (!src || dst_size == 0) {
        return;
    }
    for (size_t i = 0; src[i] != '\0' && n + 1 < dst_size; i++) {
        char c = src[i];
        if ((c >= '0' && c <= '9') || c == '-') {
            dst[n++] = c;
        }
        if (n == 10) {
            break;
        }
    }
    dst[n] = '\0';
}

static void draw_day_large(uint8_t *buf, int day)
{
    int tens = day / 10;
    int ones = day % 10;
    if (day < 10) {
        font_draw_large_digit(buf, 190, 70, ones);
    } else {
        font_draw_large_digit(buf, 170, 70, tens);
        font_draw_large_digit(buf, 192, 70, ones);
    }
}

static void draw_ascii_as_blocks(uint8_t *buf, uint16_t x, uint16_t y, const char *text)
{
    uint16_t pos = x;
    for (size_t i = 0; text && text[i] != '\0'; i++) {
        char c = text[i];
        if (c >= '0' && c <= '9') {
            font_draw_small_digit(buf, pos, y, c - '0');
            pos += 10;
        } else if (c == ':') {
            font_draw_small_colon(buf, pos, y);
            pos += 6;
        } else if (c == '-') {
            for (int k = 0; k < 6; k++) set_pixel(buf, pos + k, y + 8, true);
            pos += 8;
        } else if (c == ' ') {
            pos += 6;
        } else {
            for (int yy = 0; yy < 8; yy++) {
                for (int xx = 0; xx < 6; xx++) {
                    if (yy == 0 || yy == 7 || xx == 0 || xx == 5) {
                        set_pixel(buf, pos + xx, y + yy, true);
                    }
                }
            }
            pos += 8;
        }
        if (pos > EPD_WIDTH - 12) {
            break;
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

    clock_draw_face();
}

void clock_draw_calendar(const almanac_data_t *data)
{
    char time_buf[16] = {0};
    char date_buf[16] = {0};
    char api_date[16] = {0};
    char week_buf[16] = {0};
    int y = 0, m = 0, d = 0;
    static const char *week_name[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    static bool first_draw = true;

    app_rtc_get_time_str(time_buf, sizeof(time_buf));
    app_rtc_get_date_str(date_buf, sizeof(date_buf));

    if (data && data->date[0] != '\0') {
        parse_date(data->date, &y, &m, &d);
    }
    if (d <= 0) {
        parse_date(date_buf, &y, &m, &d);
    }

    if (data && data->date[0] != '\0') {
        extract_date_only(data->date, api_date, sizeof(api_date));
    }
    if (api_date[0] == '\0') {
        snprintf(api_date, sizeof(api_date), "%s", date_buf);
    }

    int wday = (y > 0 && m > 0 && d > 0) ? calc_weekday(y, m, d) : -1;
    if (wday >= 0 && wday <= 6) {
        snprintf(week_buf, sizeof(week_buf), "%s", week_name[wday]);
    } else {
        snprintf(week_buf, sizeof(week_buf), "N/A");
    }

    memset(display_buffer, 0xFF, sizeof(display_buffer));

    draw_rectangle(display_buffer, 2, 2, EPD_WIDTH - 4, EPD_HEIGHT - 4, false);
    draw_hline(display_buffer, 10, EPD_WIDTH - 10, 50);
    draw_hline(display_buffer, 10, EPD_WIDTH - 10, 180);
    draw_hline(display_buffer, 10, EPD_WIDTH - 10, 230);
    draw_vline(display_buffer, 130, 52, 178);
    draw_vline(display_buffer, 270, 52, 178);

    // Top info similar to Calendar_1day
    if (m > 0 && d > 0) {
        char md[32];
        int mm = (m >= 1 && m <= 12) ? m : 1;
        int dd = (d >= 1 && d <= 31) ? d : 1;
        snprintf(md, sizeof(md), "%02d-%02d", mm, dd);
        draw_centered_date(display_buffer, md, 16);
    } else {
        draw_centered_date(display_buffer, date_buf, 16);
    }

    draw_day_large(display_buffer, d > 0 ? d : 1);

    draw_ascii_as_blocks(display_buffer, 18, 188, "TIME");
    draw_ascii_as_blocks(display_buffer, 70, 188, time_buf);

    draw_ascii_as_blocks(display_buffer, 18, 208, "DATE");
    draw_ascii_as_blocks(display_buffer, 70, 208, date_buf);

    if (data) {
        draw_ascii_as_blocks(display_buffer, 18, 238, "API OK");
        draw_ascii_as_blocks(display_buffer, 100, 238, api_date);
        draw_ascii_as_blocks(display_buffer, 18, 258, "WEEK");
        draw_ascii_as_blocks(display_buffer, 100, 258, week_buf);
    } else {
        draw_ascii_as_blocks(display_buffer, 18, 238, "API EMPTY");
    }

    epd_hw_init();
    
    // First time: clear screen to white to ensure clean state
    if (first_draw) {
        epd_clear_white();
        first_draw = false;
    }
    
    epd_display_full(display_buffer);
    epd_deep_sleep();
}

void clock_draw_error(const char *title, const char *detail)
{
    memset(display_buffer, 0xFF, sizeof(display_buffer));
    draw_rectangle(display_buffer, 2, 2, EPD_WIDTH - 4, EPD_HEIGHT - 4, false);
    draw_hline(display_buffer, 10, EPD_WIDTH - 10, 60);

    draw_ascii_as_blocks(display_buffer, 20, 20, "SYSTEM ERROR");
    draw_ascii_as_blocks(display_buffer, 20, 80, title ? title : "UNKNOWN");
    draw_ascii_as_blocks(display_buffer, 20, 110, detail ? detail : "-");

    epd_hw_init();
    epd_display_full(display_buffer);
    epd_deep_sleep();
}
