#ifndef RTC_H
#define RTC_H

#include <stdint.h>
#include <stdbool.h>

// Initialize RTC/NTP time synchronization
void app_rtc_init(void);

// Get current time string (format: HH:MM:SS)
void app_rtc_get_time_str(char *buf, size_t buf_size);

// Get current date string (format: YYYY-MM-DD)
void app_rtc_get_date_str(char *buf, size_t buf_size);

// Check if time is synchronized
bool app_rtc_is_time_synced(void);

#endif // RTC_H
