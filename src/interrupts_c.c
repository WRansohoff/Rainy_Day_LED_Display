#include "interrupts_c.h"

// C-language hardware interrupt method definitions.
// TIM2 timer interrupt.
void TIM2_IRQ_handler(void) {
  // Only the 'Update' interrupt is used.
  if (LL_TIM_IsActiveFlag_UPDATE(TIM2)) {
    // Halt the timer; these are one-shots.
    //LL_TIM_DisableCounter(TIM2);
    LL_TIM_ClearFlag_UPDATE(TIM2);
    // Mark the change in state.
    //ws2812_is_writing = WS2812B_WRITING_RESERVED;
  }
  return;
}
