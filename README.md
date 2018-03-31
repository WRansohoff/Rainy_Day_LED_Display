# Overview

This is firmware for an STM32 chip to drive a colored LED 'rainy day' display. It pulses blue/green colors down a 6x24-pixel grid arranged in a zig-zag pattern.

It uses a simple timer peripheral set to tick based on the HSE oscillator signal with no prescaler as a way to generate pseudo-random numbers. The TIM3 timer seemed to be available on the broadest range of chips.

Currently tested with:

* STM32F051K8

* STM32F031F6

I'd like to add some inputs for adjusting brightness, the number of 'raindrops', and maybe the direction of the rain (vertical vs horizontal).
