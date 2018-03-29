#ifndef _VVC_GLOBAL_H
#define _VVC_GLOBAL_H

#include <stdio.h>

// Core includes.
#ifdef VVC_F0
  #include "stm32f0xx.h"
#elif VVC_F3
  #include "stm32f3xx.h"
#endif
#include "stm32_assert.h"

// LL/HAL includes.
#ifdef VVC_F0
  #include "stm32f0xx_ll_gpio.h"
  #include "stm32f0xx_ll_rcc.h"
  #include "stm32f0xx_ll_system.h"
  #include "stm32f0xx_ll_tim.h"
#elif VVC_F3
  #include "stm32f3xx_ll_gpio.h"
  #include "stm32f3xx_ll_rcc.h"
  #include "stm32f3xx_ll_system.h"
  #include "stm32f3xx_ll_tim.h"
#endif

// Struct definitions.
typedef struct {
  // [0.0 : 1.0] - intensity of the 'next' pixel.
  float next_px_progress;
  // [0.0 : 1.0] - intensity of the 'current' pixel.
  float cur_px_brightness;
  // This droplet's 'flow speed'
  float flow_speed;
  // This droplet's 'drip falloff' - higher values mean shorter
  // droplets.
  float drip_falloff;
  // Current X/Y of the 'bottom' of the droplet.
  int8_t cur_x;
  int8_t cur_y;
  // This droplet's relative max intensity. [0:255]
  uint8_t cur_intensity;
} Droplet;

// Assembly methods.
extern void delay_cycles(unsigned int d);
extern void delay_us(unsigned int d);
extern void delay_ms(unsigned int d);
extern void delay_s(unsigned int d);
extern void ws2812b_write_color(unsigned int gpio_odr,
                                unsigned int gpio_pinmask,
                                unsigned int color_grb);
extern void ws2812b_latch(unsigned int gpio_odr,
                          unsigned int gpio_pinmask);

// ----------------------
// Global variables.
uint32_t tim_counter;
// Global TIM (timer) initialization struct.
LL_TIM_InitTypeDef  timer_init_struct;

// WS2812B colored LED values.
#define LED_ARRAY_SIZE 144
volatile uint32_t led_colors_array[LED_ARRAY_SIZE];
// Array of 'droplets'
#define MAX_DROPLETS   16
volatile Droplet  raindrops[MAX_DROPLETS];

#endif
