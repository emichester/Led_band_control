# Control of a 5V Analog RGB LED Strip with ESP8266

This project controls a common-anode analog RGB LED strip using an ESP8266 board and three S8050 NPN transistors as low-side switches for the red, green, and blue channels. The current sketch connects to Wi-Fi, starts a small web server, and lets you change the strip color from any browser on the same network.

## Hardware

- 5V analog RGB LED strip (common anode), such as a 27-LED strip
- ESP8266 board, tested with a D1 mini
- 3x S8050 NPN transistors, one per color channel
- 3x 1kΩ resistors for the transistor bases
- External 5V power supply (for example, an LM2596 module or a simple phone charger) with a common ground shared with the ESP8266.

## Wiring

- D1 → 1kΩ resistor → base of the S8050 for the R channel
- D2 → 1kΩ resistor → base of the S8050 for the G channel
- D3 → 1kΩ resistor → base of the S8050 for the B channel
- Each transistor collector connects to the corresponding strip color wire (R, G, or B)
- All transistor emitters connect to the common GND
- The strip’s positive wire connects to the 5V supply

## Software

- Main sketch: Led_band_control.ino
- LED control logic is handled in utils/led_control.h
- Wi-Fi connection and reconnection are handled in utils/network.h
- The web interface is served from utils/web_server.h
- The used pins are defined on utils/gpio.h
- The confidential information is saved on utils/secrets.h

## Setup

```c++
// ----------- secrets.h -----------//
#ifndef SECRETS_H
#define SECRETS_H

// WiFi credentials
// IMPORTANT: if your repo is public, add "utils/secrets.h" to your .gitignore
// so you don't commit your real WiFi password.
#define WIFI_SSID     "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASSWORD"

#endif
```

1. Edit utils/secrets.h and set your Wi-Fi SSID and password.
2. Upload the sketch to the ESP8266.
3. Open the serial monitor to read the assigned IP address.
4. Open http://<device-ip>/ in a browser to use the color picker.

## Notes

- The web UI sends color values to the board using the /setColor endpoint and can report the current color through /status.
- PWM values are scaled to the ESP8266 range (0–1023).
- The old fade-effect example code is still present in the LED control header as a commented example, but it is not used by the main sketch.
