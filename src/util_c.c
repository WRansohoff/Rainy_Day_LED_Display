#include "util_c.h"

// C-language utility method definitions.
// Write a WS2812B LED color.
// Input should be formatted as 0x00GGRRBB.
void write_ws2812b_color(uint32_t col) {
  // Write the color 1 bit at a time.
  uint32_t col_pos = 0x00800000;
  while (col_pos > 0) {
    if (col_pos & col) {
      // Write a '1'. 0.8us high, 0.45us low. (0.02us 'tick')
      // Pull pin A0 high. TODO: constant for pin value.
      GPIOA->ODR |= 0x1;
      // 0.8us = 40 'ticks'.
      start_tim2_timer();
      while (LL_TIM_GetCounter(TIM2) < 40) {}
      LL_TIM_DisableCounter(TIM2);
      // Pull pin A0 low.
      GPIOA->ODR &= ~0x1;
      // 0.45us = 22.5 'ticks'. (use 22)
      start_tim2_timer();
      while (LL_TIM_GetCounter(TIM2) < 22) {}
      LL_TIM_DisableCounter(TIM2);
    }
    else {
      // Write a '0'. 0.4us high, 0.85us low.
      // Pull pin A0 high. TODO: constant for pin value.
      GPIOA->ODR |= 0x1;
      // 0.4us = 20 'ticks'.
      start_tim2_timer();
      while (LL_TIM_GetCounter(TIM2) < 20) {}
      LL_TIM_DisableCounter(TIM2);
      // Pull pin A0 low.
      GPIOA->ODR &= ~0x1;
      // 0.85us = 42.5 'ticks'. (use 42)
      start_tim2_timer();
      while (LL_TIM_GetCounter(TIM2) < 42) {}
      LL_TIM_DisableCounter(TIM2);
    }
    col_pos = col_pos >> 1;
  }
  return;
}

void latch_ws2812b(void) {
  // Pull pin A0 low. TODO: Pin constant.
  GPIOA->ODR &= ~0x1;
  // at 0.2us per tick, 50us would be 2500 ticks. Call it 3000.
  start_tim2_timer();
  while (LL_TIM_GetCounter(TIM2) < 3000) {}
  LL_TIM_DisableCounter(TIM2);
  // Pull pin A1 high again.
  GPIOA->ODR |= 0x1;
  return;
}

void start_tim2_timer(void) {
  // De-initialize the TIM2 timer peripheral, and then
  // re-initialize it for periodically reading from the ADC.
  // (Use TIM2 beacuse it is simple & available on most chips.)
  // TODO: Should I start breaking out functions for
  // peripheral startup?
  #ifdef VVC_F0
    // First, ensure the timer is off.
    LL_TIM_DeInit(TIM2);
    // Use a prescaler of 1, for a fast clock.
    // @48MHz PLL, this comes out to about .02 us I think.
    timer_init_struct.Prescaler = 0x0000;
    // Set the timer to count upwards.
    timer_init_struct.CounterMode = LL_TIM_COUNTERMODE_UP;
    // We won't be using 'autoreload'. Just reset at max value.
    timer_init_struct.Autoreload = 0xFFFF;
    // Set clock division to trigger on every event.
    timer_init_struct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    // Set the repetition counter to 0; unused.
    timer_init_struct.RepetitionCounter = 0x00;
    // Initialize the peripheral.
    LL_TIM_Init(TIM2, &timer_init_struct);
    // Then finally, enable the timer.
    LL_TIM_EnableCounter(TIM2);
  #elif  VVC_F3
    // First, ensure the timer is off.
    LL_TIM_DeInit(TIM2);
    // For now, set a prescaler value of 1.
    timer_init_struct.Prescaler = 0x0000;
    // Set the timer to count upwards.
    timer_init_struct.CounterMode = LL_TIM_COUNTERMODE_UP;
    // No autoreload.
    timer_init_struct.Autoreload = 0xFFFF;
    // Set clock division to trigger on every event.
    timer_init_struct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    // Set the repetition counter to 0; unused.
    timer_init_struct.RepetitionCounter = 0x00;
    // Initialize the peripheral.
    LL_TIM_Init(TIM2, &timer_init_struct);
    // Then finally, enable the timer.
    LL_TIM_EnableCounter(TIM2);
  #endif
}

// 'Waterfall' methods.
// Get the index in the LED array from a grid coordinate.
// I put an LED strip in a zig-zag pattern, so it's a bit
// complicated.
int get_grid_index(int x, int y) {
  // The grid is 6x24.
  if (x % 2 == 0) {
    // Y is indexed 'down' from 0.
    return (24*x + y);
  }
  else {
    // Y is indexed 'up' from 0.
    return (24*x + (23-y));
  }
}

void fill_grid_with_droplets(void) {
  // First, empty the color array.
  uint16_t led_counter = 0;
  for (led_counter = 0; led_counter < LED_ARRAY_SIZE; ++led_counter) {
    led_colors_array[led_counter] = 0x00000000;
  }
  // Iterate over the droplets array, ignore if Y >= 32.
  int rain_iter = 0;
  for (rain_iter = 0; rain_iter < MAX_DROPLETS; ++rain_iter) {
    if (raindrops[rain_iter].cur_y < 32) {
      // Set starting relative intensity of the 'next pixel'.
      uint8_t drop_px_intens = raindrops[rain_iter].next_px_progress * raindrops[rain_iter].cur_intensity;
      if (drop_px_intens < 0) { drop_px_intens = 0.0; }
      // Start at the 'center' and go up if necessary, with
      // decreasing intensity.
      int8_t p_y;
      float diff = 0;
      uint8_t rel_intens = raindrops[rain_iter].cur_px_brightness * raindrops[rain_iter].cur_intensity;
      for (p_y = raindrops[rain_iter].cur_y; rel_intens > 0; --p_y) {
        rel_intens = rel_intens / (1 + diff);
        diff += raindrops[rain_iter].drip_falloff;
        // Set to full brightness blue, half brightness green.
        if (p_y >= 0 && p_y < 24 && rel_intens > 1) {
          led_colors_array[get_grid_index(raindrops[rain_iter].cur_x, p_y)] +=
              (rel_intens/2 << 16) | (rel_intens);
        }
      }
      // Set the 'drop pixel', which will increase in brightness
      // until it becomes the next pixel position.
      if (raindrops[rain_iter].cur_y + 1 < 24 && drop_px_intens > 1) {
        led_colors_array[get_grid_index(raindrops[rain_iter].cur_x, raindrops[rain_iter].cur_y+1)] += (drop_px_intens/2 << 16) | (drop_px_intens);
      }
    }
  }
}

// 'Flow' a given droplet.
void flow_droplet(volatile Droplet* drop) {
  if (drop->next_px_progress < 1.0) {
    drop->next_px_progress += drop->flow_speed;
  }
  else {
    drop->cur_px_brightness -= drop->flow_speed;
    if (drop->cur_px_brightness <= 0.5) {
      // Half-brightness; it now looks like we are at the
      // 'next' pixel, so reset state and move on one.
      drop->next_px_progress = 0.0;
      drop->cur_px_brightness = 1.0;
      // Move the droplet down 1 pixel.
      ++drop->cur_y;
      /*
      if (drop->cur_y >= 28) {
        drop->cur_y = -1;
        // For now, go to the next 'X' column when reaching
        // the bottom.
        drop->cur_x++;
        if (drop->cur_x >= 6) {
          drop->cur_x = 0;
        }
      }
      */
    }
  }
}

void create_new_droplet(volatile Droplet* drop) {
  uint16_t prng_delay = LL_TIM_GetCounter(TIM2);
  prng_delay = prng_delay & 0xFF00 >> 8;
  drop->next_px_progress = 0.0;
  drop->cur_px_brightness = 1.0;
  drop->flow_speed = 0.1 + (0.65 * ((float)(prng_delay & 0x00FF) / 255.0));
  // Delay a bit to avoid having a very similar PRNG value.
  // (max. 255us) TODO: Unneccessary b/c we use least-sig bits?
  delay_us(prng_delay);
  // Pick X, Y, max intensity, drip 'length'.
  drop->cur_x = 7;
  while (drop->cur_x > 5) {
    prng_delay = LL_TIM_GetCounter(TIM2);
    drop->cur_x = prng_delay & 0x07;
  }
  drop->cur_y = -1;
  prng_delay = LL_TIM_GetCounter(TIM2);
  drop->drip_falloff = 0.4 + 1.1 * ((float)(prng_delay & 0xF) / 15.0);
  prng_delay = LL_TIM_GetCounter(TIM2);
  drop->cur_intensity = 16 + (uint8_t)(48 * ((float)(prng_delay & 0xF) / 15.0));
}
