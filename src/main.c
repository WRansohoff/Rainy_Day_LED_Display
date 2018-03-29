#include "main.h"

/**
 * Main program.
 */
int main(void) {
  // Define starting values for global variables.
  tim_counter = 0;
  // Initialize the 'raindrops' struct array.
  int rain_iter = 0;
  for (rain_iter = 0; rain_iter < MAX_DROPLETS; ++rain_iter) {
    raindrops[rain_iter].next_px_progress = 0.0;
    raindrops[rain_iter].cur_px_brightness = 1.0;
    raindrops[rain_iter].flow_speed = 0.15;
    raindrops[rain_iter].drip_falloff = 0.75;
    raindrops[rain_iter].cur_x = 0;
    raindrops[rain_iter].cur_y = 40;
    raindrops[rain_iter].cur_intensity = 32;
  }
  /*
  raindrops[0].cur_y = -1;
  raindrops[1].cur_x = 2;
  raindrops[1].cur_y = 8;
  raindrops[1].cur_intensity = 64;
  raindrops[1].flow_speed = 0.1;
  raindrops[1].drip_falloff = 0.45;
  raindrops[2].cur_x = 1;
  raindrops[2].cur_y = 18;
  raindrops[2].flow_speed = 0.25;
  raindrops[2].drip_falloff = 1.5;
  raindrops[3].cur_x = 4;
  raindrops[3].cur_y = 3;
  raindrops[3].flow_speed = 0.7;
  raindrops[4].cur_x = 5;
  raindrops[4].cur_y = 12;
  */
  // Initialize the waterfall array.
  int waterfall_x = 0;
  int waterfall_y = 0;
  uint32_t cur_color = 0x00000000;
  /*
  int cur_g = 12;
  int cur_b = 24;
  */
  for (waterfall_x = 0; waterfall_x < 6; ++waterfall_x) {
    for (waterfall_y = 0; waterfall_y < 24; ++waterfall_y) {
      /*
      cur_color = (cur_g << 16) | cur_b;
      cur_b--;
      if (cur_b < 0) { cur_b = 0; }
      if (cur_b % 2 == 0) { cur_g--; }
      if (cur_g < 0) { cur_g = 0; }
      */
      led_colors_array[get_grid_index(waterfall_x, waterfall_y)] = cur_color;
    }
    /*
    cur_b = 24;
    cur_g = 12;
    */
  }

  // Enable the GPIOA clock (use pin A0 for now.)
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
  // Enable the TIM2 clock.
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
  // Enable the SYSCFG clock for hardware interrupts.
  // TODO: is this only needed for EXTI interrupts?
  //RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
  // Enable the TIM2 clock.
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

  // Setup GPIO pin A0 as push-pull output, no pupdr,
  // 10MHz max speed.
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_0, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_0, LL_GPIO_SPEED_FREQ_MEDIUM);
  LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_0, LL_GPIO_PULL_NO);

  // Setup the TIM2 timer for PRNG.
  #ifdef VVC_F0
    // Ensure the timer is off.
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
    // TODO
  #endif

  uint16_t led_counter = 0;
  while (1) {
    // Get the current timer counter value.
    // This can act as a PRNG counter.
    tim_counter = LL_TIM_GetCounter(TIM2);

    //delay_us(250);
    fill_grid_with_droplets();
    // Flow the test droplets.
    for (rain_iter = 0; rain_iter < MAX_DROPLETS; ++rain_iter) {
      if (raindrops[rain_iter].cur_y < 32) {
        flow_droplet(&raindrops[rain_iter]);
      }
      // Generate a new droplet if it seems worthwhile.
      else {
        // This droplet is 'empty'. Maybe make a new one?
        // Say a 5% chance per cycle. 65535/20 ~= 3276.
        if (LL_TIM_GetCounter(TIM2) < 3276) {
          create_new_droplet(&raindrops[rain_iter]);
        }
      }
    }
    // Draw the waterfall grid.
    for (led_counter = 0; led_counter < 144; ++led_counter) {
      ws2812b_write_color(0x48000014, 0x1, led_colors_array[led_counter]);
    }
    //latch_ws2812b();
    ws2812b_latch(0x48000014, 0x1);
  }

  return 0;
}
