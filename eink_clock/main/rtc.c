#include "rtc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_netif.h"
#include "esp_event.h"
#include <time.h>
#include <sys/time.h>
#include "config.h"

static const char *TAG = "RTC";
static bool time_synced = false;

void time_sync_notification(struct timeval *tv)
{
    ESP_LOGI(TAG, "Time synchronized: %s", ctime(&tv->tv_sec));
    time_synced = true;
}

void app_rtc_init(void)
{
    ESP_LOGI(TAG, "Initializing RTC/NTP time synchronization");
    
    // Set SNTP servers
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, NTP_SERVER_1);
    esp_sntp_setservername(1, NTP_SERVER_2);
    esp_sntp_set_time_sync_notification_cb(time_sync_notification);
    
    // Initialize SNTP
    esp_sntp_init();
    
    ESP_LOGI(TAG, "Waiting for time synchronization...");
}

void app_rtc_get_time_str(char *buf, size_t buf_size)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    strftime(buf, buf_size, "%H:%M:%S", &timeinfo);
}

void app_rtc_get_date_str(char *buf, size_t buf_size)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    strftime(buf, buf_size, "%Y-%m-%d", &timeinfo);
}

bool app_rtc_is_time_synced(void)
{
    return time_synced;
}
