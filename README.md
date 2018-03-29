# Overview

This is firmware for an STM32 chip (currently only tested with a STM32F051K8T6) to drive a colored LED 'rainy day' display. It pulses blue/green colors down a 6x24-pixel grid arranged in a zig-zag pattern.

It uses a simple timer peripheral set to tick based on the HSE oscillator signal with no prescaler as a way to generate pseudo-random numbers. I might change that from TIM2 to TIM3 for compatibility with the STM32F030 line.
