#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "epd_driver.h"
#include "wifi.h"
#include "rtc.h"
#include "clock.h"
#include "almanac.h"
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

    almanac_data_t almanac = {0};
    bool ok = false;
    for (int i = 0; i < ALMANAC_FETCH_RETRY; i++) {
        if (almanac_fetch(&almanac)) {
            ok = true;
            break;
        }
        ESP_LOGW(TAG, "Almanac fetch failed (%d/%d): %s", i + 1, ALMANAC_FETCH_RETRY, almanac_last_error());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    if (ok) {
        clock_draw_calendar(&almanac);
    } else {
        clock_draw_error("ALMANAC FETCH FAILED", almanac_last_error());
    }

    esp_wifi_disconnect();
    esp_wifi_stop();

#if ENABLE_DEEP_SLEEP
    ESP_LOGI(TAG, "Entering deep sleep for %d minutes", SLEEP_TIME_MINUTES);
    esp_sleep_enable_timer_wakeup((uint64_t)SLEEP_TIME_MINUTES * 60ULL * 1000000ULL);
    esp_deep_sleep_start();
#endif

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
