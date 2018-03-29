#ifndef _VVC_UTIL_ASM
#define _VVC_UTIL_ASM

/*
 * Utility functions, for better legibility.
 */

.syntax unified
#ifdef VVC_F0
  .cpu cortex-m0
#elif VVC_L0
  .cpu cortex-m0plus
#elif VVC_F1
  .cpu cortex-m3
#elif  VVC_F3
  .cpu cortex-m4
#endif
.fpu softvfp
.thumb

// Instead of #include'ing this file everywhere, we can
// just declare the methods 'global' and make sure our
// Makefile compiles/links with this source file.
.global delay_cycles
.global delay_us
.global delay_ms
.global delay_s
// WS2812B functions.
.global ws2812b_write_0
.global ws2812b_write_1
.global ws2812b_write_color
.global ws2812b_latch

/*
 * Delay a given number of MCU cycles.
 * Expects:
 *  r0 contains the number of cycles to wait.
 */
.type delay_cycles,%function
.section .text.delay_cycles,"ax",%progbits
delay_cycles:
  // TODO: Currently delays (r0+4) cycles, I think.
  // PUSH/LSRS/CMP/BEQ (no branch) in the beginning.
  PUSH { r0, lr }
  // Divide the number of cycles by 4 (loop takes 4 cycles)
  LSRS r0, r0, #2
  // (Pre-loop cycle, in case 0-3 is passed in)
  // CMP, BEQ (no branch) = 2 cycles.
  CMP  r0, #0
  BEQ  delay_cycles_done
  cycle_delay_loop:
  // Subtract takes 1 instruction
  SUBS r0, r0, #1
  // SUBS already sets condition flags, but this CMP makes
  // it an even four cycles per loop.
  CMP  r0, #0
  // Branch takes 2 instructions (except the last time)
  BNE  cycle_delay_loop
  delay_cycles_done:
  POP  { r0, pc }
.size delay_cycles, .-delay_cycles

/*
 * Delay a given number of microseconds.
 * TODO: Allow for PLL clock speeds other than 48MHz.
 * Expects:
 *  r0 contains the number of microseconds to wait.
 */
.type delay_us,%function
.section .text.delay_us,"ax",%progbits
delay_us:
  PUSH { r0, r1, lr }
  // @48MHz PLL, 1 microsecond should = 48 cycles.
  MOVS r1, #48
  MULS r0, r0, r1
  BL   delay_cycles
  POP  { r0, r1, pc }
.size delay_us, .-delay_us


/*
 * Delay a given number of milliseconds.
 * TODO: Allow for PLL clock speeds other than 48MHz.
 * Expects:
 *  r0 contains the number of milliseconds to wait.
 */
.type delay_ms,%function
.section .text.delay_ms,"ax",%progbits
delay_ms:
  PUSH { r0, r1, lr }
  // @48MHz PLL, 1 millisecond should = 48,000 cycles.
  LDR  r1, =0x0000BB80
  MULS r0, r0, r1
  BL   delay_cycles
  POP  { r0, r1, pc }
.size delay_ms, .-delay_ms


/*
 * Delay a given number of seconds.
 * TODO: Allow for PLL clock speeds other than 48MHz.
 * Expects:
 *  r0 contains the number of seconds to wait.
 */
.type delay_s,%function
.section .text.delay_s,"ax",%progbits
delay_s:
  PUSH { r0, r1, lr }
  // @48MHz PLL, 1 second should = 48,000,000 cycles.
  LDR  r1, =0x02DC6C00
  MULS r0, r0, r1
  BL   delay_cycles
  POP  { r0, r1, pc }
.size delay_s, .-delay_s

/*
 * Write a '0' to a WS2812B control pin. This means:
 * 'high' for 0.40 us, 'low' for 0.85us.
 * TODO: Support PLL clock values other than 48MHz.
 * (Also, these timings are inferred from a 'scope;
 *  come up with a more robust system.)
 * Expects:
 *  r0 contains the GPIO ODR address.
 *  r1 contains the GPIO pin mask.
 */
.type ws2812b_write_0,%function
.section .text.ws2812b_write_0,"ax",%progbits
ws2812b_write_0:
  PUSH { r0, r1, r2, r6, r7, lr }
  MOVS r6, r0
  MOVS r7, r1
  // Pull the pin high.
  LDR  r2, [r6]
  ORRS r2, r2, r7
  STR  r2, [r6]
  // Delay ~0.35 us.
  // (Just wait several cycles.)
  MOVS r0, #6
  ws2812b_write_0_bit_delay:
    SUBS r0, #3
    BNE  ws2812b_write_0_bit_delay
  // Pull the pin low.
  LDR  r2, [r6]
  MVNS r7, r7
  ANDS r2, r2, r7
  STR  r2, [r6]
  // Delay ~0.9 us.
  // (Jumping between functions introduces some extra time...)
  MOVS r0, #12
  BL   delay_cycles
  POP  { r0, r1, r2, r6, r7, pc }
.size ws2812b_write_0, .-ws2812b_write_0

/*
 * Write a '1' to a WS2812B control pin. This means:
 * 'high' for 0.80 us, 'low' for 0.45us.
 * TODO: Support PLL clock values other than 48MHz.
 * (Also, these timings are inferred from a 'scope;
 *  come up with a more robust system.)
 * Expects:
 *  r0 contains the GPIO ODR address.
 *  r1 contains the GPIO pin mask.
 */
.type ws2812b_write_1,%function
.section .text.ws2812b_write_1,"ax",%progbits
ws2812b_write_1:
  PUSH { r0, r1, r2, r6, r7, lr }
  MOVS r6, r0
  MOVS r7, r1
  // Pull the pin high.
  LDR  r2, [r6]
  ORRS r2, r2, r7
  STR  r2, [r6]
  // Delay ~0.90 us.
  MOVS r0, #12
  BL   delay_cycles
  // Pull the pin low.
  LDR  r2, [r6]
  MVNS r7, r7
  ANDS r2, r2, r7
  STR  r2, [r6]
  // Delay ~0.35 us.
  MOVS r0, #9
  ws2812b_write_1_bit_delay:
    SUBS r0, #3
    BNE  ws2812b_write_1_bit_delay
  BL   delay_cycles
  POP  { r0, r1, r2, r6, r7, pc }
.size ws2812b_write_1, .-ws2812b_write_1

/*
 * Write a 24-bit color to a WS2812 pin.
 * Expects:
 *  r0 contains the GPIO ODR address.
 *  r1 contains the GPIO pin mask.
 *  r2 contains the color to write, from 0x00000000
 *     up to 0x00GGRRBB
 */
.type ws2812b_write_color,%function
.section .text.ws2812b_write_color,"ax",%progbits
ws2812b_write_color:
  PUSH { r0, r1, r2, r3, r4, r5, lr }
  LDR  r3, =0x00800000
  send_ws2812b_color_bit:
    // Check the current bit.
    MOVS r4, r2
    ANDS r4, r4, r3
    BEQ  ws2812b_write_color_bit_0
    // (Write a '1')
    BL   ws2812b_write_1
    B    ws2812b_bit_done
    ws2812b_write_color_bit_0:
    BL   ws2812b_write_0
    //B    ws2812b_bit_done
    ws2812b_bit_done:
      LSRS r3, r3, #1
      BEQ ws2812b_write_color_done
      B   send_ws2812b_color_bit
  ws2812b_write_color_done:
  POP  { r0, r1, r2, r3, r4, r5, pc }
.size ws2812b_write_color, .-ws2812b_write_color

/*
 * Latch a WS2812B control pin to set previously-
 * written colors. This just means delaying for >= 50us.
 * Expects:
 *  r0 contains the GPIO ODR address.
 *  r1 contains the GPIO pin mask.
 */
.type ws2812b_latch,%function
.section .text.ws2812b_latch,"ax",%progbits
ws2812b_latch:
  PUSH { r0, r1, r2, r6, r7, lr }
  MOVS r6, r0
  MOVS r7, r1
  // Pull the pin low.
  LDR  r2, [r6]
  MVNS r7, r7
  ANDS r2, r2, r7
  STR  r2, [r6]
  // Delay 60 us.
  MOVS r0, #60
  BL   delay_us
  POP  { r0, r1, r2, r6, r7, pc }
.size ws2812b_latch, .-ws2812b_latch

#endif
