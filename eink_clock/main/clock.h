#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

// Initialize clock display
void clock_init(void);

// Update clock display with current time
void clock_update_display(void);

// Draw clock face (background)
void clock_draw_face(void);

#endif // CLOCK_H
