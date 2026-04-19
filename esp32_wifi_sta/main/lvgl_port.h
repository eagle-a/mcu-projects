#ifndef LVGL_PORT_H
#define LVGL_PORT_H

#include "lvgl.h"

// 初始化 LVGL 显示驱动
void lvgl_port_init(void);

// LVGL 定时器任务
void lvgl_timer_task(void *arg);

#endif // LVGL_PORT_H
