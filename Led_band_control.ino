/*
  RGB LED Strip Control - Main sketch
  -------------------------------------
  Board: D1 mini (ESP8266)

  Connects to WiFi (with auto-reconnect), starts a web server to control
  the strip color remotely, restores the last saved color on boot, and
  supports a continuous color-rotation mode.

  Wiring:
    D1 -> 1k resistor -> base of S8050 (R channel) -> collector -> strip R
    D2 -> 1k resistor -> base of S8050 (G channel) -> collector -> strip G
    D3 -> 1k resistor -> base of S8050 (B channel) -> collector -> strip B
    Strip "+" -> external 5V supply (LM2596 output)
    GND: supply, board and all 3 transistor emitters tied together

  Before uploading:
    Edit utils/secrets.h with your WiFi SSID and password.
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

  connectToWiFi();
  webServerInit();
}

void loop() {
  maintainWiFiConnection();
  webServerLoop();
  ledEffectsLoop();
  maintainColorPersistence();
}
