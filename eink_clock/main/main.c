#include <stdio.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "epd_driver.h"
#include "wifi.h"
#include "rtc.h"
#include "clock.h"
#include "config.h"

static const char *TAG = "APP_MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-C3 E-Ink Clock Starting");

    // Initialize NVS
    ESP_ERROR_CHECK(nvs_flash_init());

    // Initialize WiFi
    ESP_LOGI(TAG, "Initializing WiFi...");
    wifi_init_sta();

    // Wait for WiFi to connect and time to sync
    vTaskDelay(pdMS_TO_TICKS(5000));

    // Initialize RTC/NTP
    ESP_LOGI(TAG, "Initializing RTC/NTP...");
    app_rtc_init();

    // Wait for time synchronization
    ESP_LOGI(TAG, "Waiting for time synchronization...");
    int wait_count = 0;
    while (!app_rtc_is_time_synced() && wait_count < NTP_SYNC_TIMEOUT_SEC) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        wait_count++;
        ESP_LOGI(TAG, "Waiting for time sync... (%d/%d)", wait_count, NTP_SYNC_TIMEOUT_SEC);
    }

    if (app_rtc_is_time_synced()) {
        ESP_LOGI(TAG, "Time synchronized successfully");
    } else {
        ESP_LOGW(TAG, "Time synchronization timeout, using default time");
    }

    // Initialize e-ink display
    ESP_LOGI(TAG, "Initializing e-ink display...");
    epd_init();

    // Initialize clock display
    ESP_LOGI(TAG, "Initializing clock...");
    clock_init();

    // Main loop - update display every minute
    ESP_LOGI(TAG, "Starting main loop - update every %d seconds", CLOCK_UPDATE_INTERVAL_SEC);
    
    time_t last_minute = 0;
    int update_count = 0;
    
    while (1) {
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        
        // Update at the start of each minute
        if (timeinfo.tm_min != last_minute) {
            last_minute = timeinfo.tm_min;
            update_count++;
            
            ESP_LOGI(TAG, "Updating display (update #%d)", update_count);
            clock_update_display();
            
            // Perform full refresh every N updates to clear ghosting
            if (update_count % CLOCK_FULL_REFRESH_COUNT == 0) {
                ESP_LOGI(TAG, "Performing full refresh to clear ghosting");
                clock_draw_face();
                clock_update_display();
            }
            
#if ENABLE_DEEP_SLEEP
            // Enter deep sleep to save power
            ESP_LOGI(TAG, "Entering deep sleep for %d seconds", CLOCK_UPDATE_INTERVAL_SEC - 10);
            esp_sleep_enable_timer_wakeup((CLOCK_UPDATE_INTERVAL_SEC - 10) * 1000000);
            esp_deep_sleep_start();
#endif
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
