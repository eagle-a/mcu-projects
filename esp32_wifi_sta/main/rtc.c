#include "rtc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_netif.h"
#include "esp_event.h"
#include <time.h>
#include <sys/time.h>

static const char *TAG = "RTC";
static bool time_synced = false;

void time_sync_notification(struct timeval *tv)
{
    ESP_LOGI(TAG, "时间同步完成: %s", ctime(&tv->tv_sec));
    time_synced = true;
}

void app_rtc_init(void)
{
    ESP_LOGI(TAG, "初始化 RTC/NTP 时间同步");
    
    // 设置 SNTP 服务器
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.nist.gov");
    esp_sntp_set_time_sync_notification_cb(time_sync_notification);
    
    // 初始化 SNTP
    esp_sntp_init();
    
    ESP_LOGI(TAG, "等待时间同步...");
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
