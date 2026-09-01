/*
  RGB LED Strip Control - Main sketch
  -------------------------------------
  Board: D1 mini (ESP8266)

  Runs as its own WiFi access point (no router needed). The Android app
  connects directly to it to control the strip. Restores the last saved
  color on boot and supports a continuous color-rotation mode.

  Wiring:
    D1 -> 1k resistor -> base of S8050 (R channel) -> collector -> strip R
    D2 -> 1k resistor -> base of S8050 (G channel) -> collector -> strip G
    D3 -> 1k resistor -> base of S8050 (B channel) -> collector -> strip B
    Strip "+" -> external 5V supply (LM2596 output)
    GND: supply, board and all 3 transistor emitters tied together

  Before uploading:
    Edit utils/secrets.h with the AP name/password you want.
    Make sure the Android app's AP_SSID/AP_PASSWORD constants match.
*/

#include "utils/network.h"
#include "utils/led_control.h"
#include "utils/storage.h"
#include "utils/persistence.h"
#include "utils/web_server.h"

void setup() {
  Serial.begin(115200);

  ledControlInit();
  storageInit();
  restoreLastColor();

  startAccessPoint();
  webServerInit();
}

void loop() {
  webServerLoop();
  ledEffectsLoop();
  maintainColorPersistence();
}