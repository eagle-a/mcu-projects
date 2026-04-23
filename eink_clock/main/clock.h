#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>
#include "almanac.h"

// Initialize clock display
void clock_init(void);

// Update clock display with current time
void clock_update_display(void);

// Draw clock face (background)
void clock_draw_face(void);

// Render a calendar page using fetched almanac data
void clock_draw_calendar(const almanac_data_t *data);

// Render error page when fetch/sync fails
void clock_draw_error(const char *title, const char *detail);

#endif // CLOCK_H
