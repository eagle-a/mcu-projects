#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/

/*Color depth: 1 (1 byte per pixel), 8 (RGB332), 16 (RGB565), 32 (ARGB8888)*/
#define LV_COLOR_DEPTH 16

/*=========================
   MEMORY SETTINGS
 *=========================*/

/*Size of the memory available for `lv_malloc()` in bytes (>= 2kB)*/
#define LV_MEM_SIZE (16U * 1024U)  /* 16 KB */

/*Set an address for memory pool instead of allocating it as a normal array*/
#define LV_MEM_ADR 0

/*Use the standard `memcpy` and `memset` instead of LVGL's own functions*/
#define LV_MEMCPY_MEMSET_STD 1

/*====================
   HAL SETTINGS
 *====================*/

/*Default display refresh period in milliseconds*/
#define LV_DEF_REFR_PERIOD 30

/*Input device read period in milliseconds*/
#define LV_INDEV_DEF_READ_PERIOD 30

/*Use a custom tick source that tells the elapsed time in milliseconds*/
#define LV_TICK_CUSTOM 0

/*==================
   OPERATING SYSTEM
 *==================*/

/*Select an operating system to use. Possible options:
 * - LV_OS_NONE
 * - LV_OS_FREERTOS
 * - LV_OS_RTTHREAD
 * - LV_OS_WINDOWS
 * - LV_OS_PTHREAD
 * - LV_OS_CMSIS_RTOS2
 * - LV_OS_CUSTOM */
#define LV_USE_OS LV_OS_FREERTOS

/*===================
   FONT USAGE
 *===================*/

/*Montserrat fonts with various sizes*/
#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 0
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 0
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 0
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 0
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

/*==================
   WIDGET USAGE
 *==================*/

/*Enable the built-in widgets*/
#define LV_USE_ANIMIMG    0
#define LV_USE_BAR        0
#define LV_USE_BUTTON     1
#define LV_USE_CALENDAR   0
#define LV_USE_CANVAS     0
#define LV_USE_CHART      0
#define LV_USE_CHECKBOX   0
#define LV_USE_DROPDOWN   0
#define LV_USE_IMAGE      1
#define LV_USE_IMAGEBUTTON 0
#define LV_USE_KEYBOARD   0
#define LV_USE_LABEL      1
#define LV_USE_LED        0
#define LV_USE_LINE       0
#define LV_USE_LIST       0
#define LV_USE_MENU       0
#define LV_USE_MSGBOX     0
#define LV_USE_ROLLER     0
#define LV_USE_SCALE      0
#define LV_USE_SLIDER     0
#define LV_USE_SPAN       0
#define LV_USE_SPINBOX    0
#define LV_USE_SPINNER    0
#define LV_USE_SWITCH     0
#define LV_USE_TEXTAREA   0
#define LV_USE_TABLE      0
#define LV_USE_TABVIEW    0
#define LV_USE_TILEVIEW   0
#define LV_USE_WIN        0

/*==================
   LAYOUTS
 *==================*/

/*A layout describes how to arrange widgets in a display*/
#define LV_USE_FLEX 1
#define LV_USE_GRID 1

/*==================
   OTHERS
 *==================*/

/*1: Enable API to log events*/
#define LV_USE_LOG 1

/*1: Enable the built-in log module*/
#define LV_LOG_PRINTF 1

#endif /*LV_CONF_H*/
