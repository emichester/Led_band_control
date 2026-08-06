#ifndef GPIO_H
#define GPIO_H

#if defined(ESP8266)
  // Pin definitions for D1 mini (ESP8266)
  // Each pin drives the base (through a 1k resistor) of an S8050 transistor
  // acting as a switch for that color channel. 
  #define PIN_R D1
  #define PIN_G D2
  #define PIN_B D3
  // ESP8266 PWM resolution (analogWrite goes 0-1023 by default, but we set it
  // explicitly here so the code is easy to read and to port to other boards)
  #define PWM_MAX 1023
#else
  #define PIN_R 9
  #define PIN_G 10
  #define PIN_B 11
  #define PWM_MAX 255
#endif

#endif