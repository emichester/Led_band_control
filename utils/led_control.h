#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>
#include "gpio.h"

struct RGBColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

static RGBColor currentColor = {0, 0, 0};

inline void ledControlInit() {
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  analogWriteRange(PWM_MAX);
  analogWriteFreq(1000); // 1kHz PWM, avoids visible flicker
}

// Set the strip color. Input values use the standard 0-255 web color range,
// internally mapped to the board's PWM resolution.
inline void ledSetColor(uint8_t r, uint8_t g, uint8_t b) {
  currentColor = {r, g, b};
  analogWrite(PIN_R, map(r, 0, 255, 0, PWM_MAX));
  analogWrite(PIN_G, map(g, 0, 255, 0, PWM_MAX));
  analogWrite(PIN_B, map(b, 0, 255, 0, PWM_MAX));
}

inline RGBColor ledGetColor() {
  return currentColor;
}

inline void ledOff() {
  ledSetColor(0, 0, 0);
}

// ---------------------------------------------------------------- //
//   The following code could be used for a rotating color effect   //
// ---------------------------------------------------------------- //

// r, g, b de 0.0 a 1.0
void setColor(float r, float g, float b) {
  analogWrite(PIN_R, (int)(r * PWM_MAX));
  analogWrite(PIN_G, (int)(g * PWM_MAX));
  analogWrite(PIN_B, (int)(b * PWM_MAX));
}

// Transición suave entre varios colores (efecto "fade")
void fundidoColores() {
  static float colores[][3] = {
    {1, 0, 0},   // rojo
    {0, 1, 0},   // verde
    {0, 0, 1},   // azul
    {1, 1, 0},   // amarillo
    {0, 1, 1},   // cian
    {1, 0, 1},   // magenta
  };
  static int numColores = 6;

  for (int c = 0; c < numColores; c++) {
    int siguiente = (c + 1) % numColores;
    for (int paso = 0; paso <= 100; paso++) {
      float t = paso / 100.0;
      float r = colores[c][0] + (colores[siguiente][0] - colores[c][0]) * t;
      float g = colores[c][1] + (colores[siguiente][1] - colores[c][1]) * t;
      float b = colores[c][2] + (colores[siguiente][2] - colores[c][2]) * t;
      setColor(r, g, b);
      delay(20);
    }
  }
}

#endif