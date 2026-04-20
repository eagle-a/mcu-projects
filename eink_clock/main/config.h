#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "350love"
#define WIFI_PASS "1123581321"
#define WIFI_MAX_RETRY 5

// NTP Configuration
#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "time.nist.gov"
#define NTP_SYNC_TIMEOUT_SEC 30

// Display Configuration
#define EPD_WIDTH   400 
#define EPD_HEIGHT  300

// Pin Configuration for ESP32-C3 (基于CORE ESP32C3开发板)
#define EPD_BUSY_PIN    8   // IO08
#define EPD_RST_PIN     5   // IO05
#define EPD_DC_PIN      6   // IO06
#define EPD_CS_PIN      7   // IO07

// SPI Pin Configuration (SPI2_HOST)
#define EPD_SPI_MOSI_PIN 3   // IO03 - SPI2_MOSI
#define EPD_SPI_SCK_PIN  2   // IO02 - SPI2_CK

// SPI Configuration
#define EPD_SPI_HOST    SPI2_HOST
#define EPD_SPI_FREQ    10000000  // 10 MHz

// Clock Configuration
#define CLOCK_UPDATE_INTERVAL_SEC 60  // Update every minute
#define CLOCK_FULL_REFRESH_COUNT 5    // Full refresh every N updates

// Power Management
#define ENABLE_DEEP_SLEEP 0  // Set to 1 to enable deep sleep between updates

#endif // CONFIG_H
