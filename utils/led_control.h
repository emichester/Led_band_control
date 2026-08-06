#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>
#include "gpio.h"

struct RGBColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

enum LedMode {
  MODE_STATIC,
  MODE_ROTATE
};
 
static RGBColor currentColor = {0, 0, 0};
static LedMode currentMode = MODE_STATIC;

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


inline void ledSetMode(LedMode mode) {
  currentMode = mode;
}
 
inline LedMode ledGetMode() {
  return currentMode;
}
 
// --- Color rotation effect ---
 
// Time between hue steps in ms. Lower = faster rotation.
#define ROTATE_STEP_MS 30
 
static unsigned long lastRotateStep = 0;
static uint8_t rotateWheelPos = 0;
 
// Classic 0-255 color wheel (same pattern used in most NeoPixel examples).
// Walking 'pos' from 0 to 255 sweeps smoothly through the full rainbow.
inline RGBColor colorWheel(uint8_t pos) {
  RGBColor c;
  pos = 255 - pos;
  if (pos < 85) {
    c.r = 255 - pos * 3;
    c.g = 0;
    c.b = pos * 3;
  } else if (pos < 170) {
    pos -= 85;
    c.r = 0;
    c.g = pos * 3;
    c.b = 255 - pos * 3;
  } else {
    pos -= 170;
    c.r = pos * 3;
    c.g = 255 - pos * 3;
    c.b = 0;
  }
  return c;
}
 
// Call every loop() iteration. Advances the rainbow rotation while
// MODE_ROTATE is active; does nothing otherwise.
inline void ledEffectsLoop() {
  if (currentMode != MODE_ROTATE) return;
 
  unsigned long now = millis();
  if (now - lastRotateStep < ROTATE_STEP_MS) return;
  lastRotateStep = now;
 
  rotateWheelPos++; // uint8_t wraps 255 -> 0 automatically
  RGBColor c = colorWheel(rotateWheelPos);
  ledSetColor(c.r, c.g, c.b);
}
 
#endif