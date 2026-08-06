#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "led_control.h"
#include "storage.h"

// How long the color must stay unchanged before it gets written to flash.
// Prevents wearing out the EEPROM if the user drags the color picker.
#define COLOR_SAVE_DELAY 2000

static unsigned long lastColorChangeTime = 0;
static bool colorSavePending = false;

// Call this whenever the color is changed manually (e.g. from the web
// handler). It does NOT write to flash immediately, it just marks that
// a save is due after COLOR_SAVE_DELAY ms of no further changes.
inline void scheduleColorSave() {
  colorSavePending = true;
  lastColorChangeTime = millis();
}

// Call this every loop() iteration.
inline void maintainColorPersistence() {
  if (!colorSavePending) return;
  if (millis() - lastColorChangeTime < COLOR_SAVE_DELAY) return;

  saveColorToStorage(ledGetColor());
  colorSavePending = false;
  Serial.println("Color saved to flash");
}

// Call once from setup(), after ledControlInit() and storageInit().
inline void restoreLastColor() {
  RGBColor c;
  if (loadColorFromStorage(c)) {
    ledSetColor(c.r, c.g, c.b);
    Serial.println("Restored last saved color");
  }
}

#endif