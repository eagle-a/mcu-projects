#ifndef FONT_H
#define FONT_H

#include <stdint.h>

// Large digit font (16x32 pixels) for time display
extern const uint8_t font_large_digits[10][128];

// Medium digit font (12x24 pixels) for date display
extern const uint8_t font_medium_digits[10][36];

// Small digit font (8x16 pixels) for status display
extern const uint8_t font_small_digits[10][16];

// Colon bitmap (8x32 pixels)
extern const uint8_t font_large_colon[32];

// Colon bitmap (6x24 pixels)
extern const uint8_t font_medium_colon[24];

// Colon bitmap (4x16 pixels)
extern const uint8_t font_small_colon[16];

// Helper functions to draw characters
void font_draw_large_digit(uint8_t *buf, uint16_t x, uint16_t y, uint8_t digit);
void font_draw_medium_digit(uint8_t *buf, uint16_t x, uint16_t y, uint8_t digit);
void font_draw_small_digit(uint8_t *buf, uint16_t x, uint16_t y, uint8_t digit);

void font_draw_large_colon(uint8_t *buf, uint16_t x, uint16_t y);
void font_draw_medium_colon(uint8_t *buf, uint16_t x, uint16_t y);
void font_draw_small_colon(uint8_t *buf, uint16_t x, uint16_t y);

#endif // FONT_H
