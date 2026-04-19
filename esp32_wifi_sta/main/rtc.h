#ifndef RTC_H
#define RTC_H

#include <stdint.h>
#include <stdbool.h>

// 初始化 RTC/NTP 时间同步
void app_rtc_init(void);

// 获取当前时间字符串 (格式: HH:MM:SS)
void app_rtc_get_time_str(char *buf, size_t buf_size);

// 获取当前日期字符串 (格式: YYYY-MM-DD)
void app_rtc_get_date_str(char *buf, size_t buf_size);

// 检查时间是否已同步
bool app_rtc_is_time_synced(void);

#endif // RTC_H
