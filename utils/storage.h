#ifndef STORAGE_H
#define STORAGE_H

#include <EEPROM.h>
#include "led_control.h"

#define EEPROM_SIZE 8
#define EEPROM_MAGIC 0xA5 // marks that a valid color has been saved before

inline void storageInit() {
  EEPROM.begin(EEPROM_SIZE);
}

inline void saveColorToStorage(const RGBColor &color) {
  EEPROM.write(0, EEPROM_MAGIC);
  EEPROM.write(1, color.r);
  EEPROM.write(2, color.g);
  EEPROM.write(3, color.b);
  EEPROM.commit();
}

// Returns true and fills 'color' if a previously saved value was found.
// Returns false on first boot (nothing saved yet).
inline bool loadColorFromStorage(RGBColor &color) {
  if (EEPROM.read(0) != EEPROM_MAGIC) {
    return false;
  }
  color.r = EEPROM.read(1);
  color.g = EEPROM.read(2);
  color.b = EEPROM.read(3);
  return true;
}

#endif