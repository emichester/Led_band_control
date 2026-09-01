#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>

// Pin definitions for D1 mini (ESP8266)
#define PIN_R D1
#define PIN_G D2
#define PIN_B D3

#define PWM_MAX 1023

struct RGBColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

enum LedMode {
  MODE_STATIC,
  MODE_SMOOTH, // continuous rainbow hue rotation
  MODE_FADE,   // slow crossfade between a fixed palette of colors
  MODE_FLASH,  // jumps between palette colors abruptly
  MODE_STROBE  // rapid on/off blink
};

static RGBColor currentColor = {0, 0, 0}; // base/target color, before brightness scaling
static LedMode currentMode = MODE_STATIC;
static uint8_t brightness = 100; // percentage, 0-100

inline void ledControlInit() {
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  analogWriteRange(PWM_MAX);
  analogWriteFreq(1000); // 1kHz PWM, avoids visible flicker
}

// Sets the strip color (0-255 per channel). This is the base/target color;
// the actual PWM output is this color scaled by the current brightness.
inline void ledSetColor(uint8_t r, uint8_t g, uint8_t b) {
  currentColor = {r, g, b};

  uint8_t outR = (uint16_t)r * brightness / 100;
  uint8_t outG = (uint16_t)g * brightness / 100;
  uint8_t outB = (uint16_t)b * brightness / 100;

  analogWrite(PIN_R, map(outR, 0, 255, 0, PWM_MAX));
  analogWrite(PIN_G, map(outG, 0, 255, 0, PWM_MAX));
  analogWrite(PIN_B, map(outB, 0, 255, 0, PWM_MAX));
}

inline RGBColor ledGetColor() {
  return currentColor;
}

inline void ledOff() {
  ledSetColor(0, 0, 0);
}

// --- Brightness ---

inline void ledSetBrightness(int level) {
  brightness = constrain(level, 0, 100);
  ledSetColor(currentColor.r, currentColor.g, currentColor.b); // re-apply at new brightness
}

inline void ledAdjustBrightness(int delta) {
  ledSetBrightness((int)brightness + delta);
}

inline uint8_t ledGetBrightness() {
  return brightness;
}

// --- Mode state ---

inline LedMode ledGetMode() {
  return currentMode;
}

// Shared palette used by FADE and FLASH modes.
static const RGBColor palette[] = {
  {255, 0, 0}, {255, 140, 0}, {255, 255, 0}, {0, 255, 0},
  {0, 255, 255}, {0, 0, 255}, {150, 0, 255}, {255, 0, 255}
};
#define PALETTE_SIZE (sizeof(palette) / sizeof(palette[0]))

// --- MODE_SMOOTH: continuous rainbow ---
#define SMOOTH_STEP_MS 30
static unsigned long lastSmoothStep = 0;
static uint8_t smoothWheelPos = 0;

inline RGBColor colorWheel(uint8_t pos) {
  RGBColor c;
  pos = 255 - pos;
  if (pos < 85) {
    c.r = 255 - pos * 3; c.g = 0; c.b = pos * 3;
  } else if (pos < 170) {
    pos -= 85;
    c.r = 0; c.g = pos * 3; c.b = 255 - pos * 3;
  } else {
    pos -= 170;
    c.r = pos * 3; c.g = 255 - pos * 3; c.b = 0;
  }
  return c;
}

inline void ledSmoothLoop() {
  unsigned long now = millis();
  if (now - lastSmoothStep < SMOOTH_STEP_MS) return;
  lastSmoothStep = now;
  smoothWheelPos++; // uint8_t wraps 255 -> 0
  RGBColor c = colorWheel(smoothWheelPos);
  ledSetColor(c.r, c.g, c.b);
}

// --- MODE_FADE: smooth crossfade across the palette ---
#define FADE_TRANSITION_MS 3000
static unsigned long fadeStartTime = 0;
static uint8_t fadeIndex = 0;

inline void ledFadeLoop() {
  unsigned long now = millis();
  if (fadeStartTime == 0) fadeStartTime = now;

  float t = (float)(now - fadeStartTime) / FADE_TRANSITION_MS;
  if (t >= 1.0f) {
    fadeIndex = (fadeIndex + 1) % PALETTE_SIZE;
    fadeStartTime = now;
    t = 0.0f;
  }

  RGBColor a = palette[fadeIndex];
  RGBColor b = palette[(fadeIndex + 1) % PALETTE_SIZE];
  ledSetColor(
    a.r + (uint8_t)((b.r - a.r) * t),
    a.g + (uint8_t)((b.g - a.g) * t),
    a.b + (uint8_t)((b.b - a.b) * t)
  );
}

// --- MODE_FLASH: abrupt jumps across the palette ---
#define FLASH_HOLD_MS 400
static unsigned long lastFlashChange = 0;
static uint8_t flashIndex = 0;

inline void ledFlashLoop() {
  unsigned long now = millis();
  if (now - lastFlashChange < FLASH_HOLD_MS) return;
  lastFlashChange = now;
  flashIndex = (flashIndex + 1) % PALETTE_SIZE;
  RGBColor c = palette[flashIndex];
  ledSetColor(c.r, c.g, c.b);
}

// --- MODE_STROBE: rapid on/off blink of the color that was active when the mode started ---
#define STROBE_INTERVAL_MS 80
static unsigned long lastStrobeToggle = 0;
static bool strobeOn = true;
static RGBColor strobeBaseColor = {255, 255, 255};

inline void ledStrobeLoop() {
  unsigned long now = millis();
  if (now - lastStrobeToggle < STROBE_INTERVAL_MS) return;
  lastStrobeToggle = now;
  strobeOn = !strobeOn;
  if (strobeOn) {
    ledSetColor(strobeBaseColor.r, strobeBaseColor.g, strobeBaseColor.b);
  } else {
    ledSetColor(0, 0, 0);
  }
}

// Switches mode and resets each effect's internal timing so it starts
// cleanly instead of jumping mid-cycle from leftover state.
inline void ledSetMode(LedMode mode) {
  currentMode = mode;
  unsigned long now = millis();

  switch (mode) {
    case MODE_SMOOTH:
      lastSmoothStep = now;
      break;
    case MODE_FADE:
      fadeStartTime = 0;
      fadeIndex = 0;
      break;
    case MODE_FLASH:
      lastFlashChange = 0;
      flashIndex = 0;
      break;
    case MODE_STROBE: {
      bool hasColor = currentColor.r || currentColor.g || currentColor.b;
      strobeBaseColor = hasColor ? currentColor : RGBColor{255, 255, 255};
      lastStrobeToggle = now;
      strobeOn = true;
      ledSetColor(strobeBaseColor.r, strobeBaseColor.g, strobeBaseColor.b);
      break;
    }
    default:
      break;
  }
}

// Call every loop() iteration. Dispatches to whichever effect is active;
// does nothing while MODE_STATIC.
inline void ledEffectsLoop() {
  switch (currentMode) {
    case MODE_SMOOTH: ledSmoothLoop(); break;
    case MODE_FADE:   ledFadeLoop();   break;
    case MODE_FLASH:  ledFlashLoop();  break;
    case MODE_STROBE: ledStrobeLoop(); break;
    default: break;
  }
}

#endif