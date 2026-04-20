#ifndef EPD_DRIVER_H
#define EPD_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

#define EPD_ARRAY   (EPD_WIDTH * EPD_HEIGHT / 8)

// Initialize the e-ink display driver
void epd_init(void);

// Full screen refresh initialization
void epd_hw_init(void);

// Fast refresh initialization
void epd_hw_init_fast(void);

// Display full screen image
void epd_display_full(const uint8_t *datas);

// Clear screen to white
void epd_clear_white(void);

// Clear screen to black
void epd_clear_black(void);

// Partial refresh - set base map
void epd_set_base_map(const uint8_t *datas);

// Partial refresh at specific location
void epd_display_partial(uint16_t x_start, uint16_t y_start, 
                        const uint8_t *datas, uint16_t part_column, uint16_t part_line);

// Enter deep sleep mode
void epd_deep_sleep(void);

// Check if display is busy
bool epd_is_busy(void);

#endif // EPD_DRIVER_H
