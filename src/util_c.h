#ifndef _VVC_UTIL_C_H
#define _VVC_UTIL_C_H

#include "global.h"

// C-languages utility method signatures.
// Write a WS2812B LED color.
void write_ws2812b_color(uint32_t col);
// Latch a set of WS2812B LEDs to apply the written colors.
void latch_ws2812b(void);
// Set a timer interrupt to trigger after N ticks:
void start_tim2_timer(void);

// 'Waterfall'-related methods.
int get_grid_index(int x, int y);
// Fill the LED grid with colors based on the current
// state of the global 'Droplet' structs.
void fill_grid_with_droplets(void);
// 'Flow' a given droplet.
void flow_droplet(volatile Droplet* drop);
// Fill a given Droplet struct with data for a new random droplet.
void create_new_droplet(volatile Droplet* drop);

#endif
