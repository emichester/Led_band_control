# Control of a 5V Analog RGB LED Strip with ESP8266

This project controls a common-anode analog RGB LED strip with an ESP8266 board and three S8050 transistors. The firmware connects to Wi-Fi, serves a small web interface, and supports manual color control plus a continuous color-rotation mode.

## Features

- Remote color control from a web page
- A rotate/stop button for a rainbow-style color cycle
- Automatic Wi-Fi reconnection if the connection drops
- Last selected color restored after reboot using EEPROM persistence
- Simple transistor-based switching for the RGB channels

## Hardware

- 5V analog RGB LED strip (common anode), such as a 27-LED strip
- ESP8266 board, tested with a D1 mini
- 3x S8050 NPN transistors, one per color channel
- 3x 1kΩ resistors for the transistor bases
- External 5V power supply, such as an LM2596 module or a phone charger, with a shared ground

## Wiring

- D1 → 1kΩ resistor → base of the S8050 for the R channel
- D2 → 1kΩ resistor → base of the S8050 for the G channel
- D3 → 1kΩ resistor → base of the S8050 for the B channel
- Each transistor collector connects to the corresponding strip color wire (R, G, or B)
- All transistor emitters connect to the common GND
- The strip’s positive wire connects to the 5V supply

## Software

- Led_band_control.ino — main sketch
- utils/led_control.h — PWM control, color handling, and rotation effect
- utils/storage.h — EEPROM read/write access
- utils/persistence.h — delayed color saving and restore-on-boot logic
- utils/network.h — Wi-Fi connection and reconnection
- utils/web_server.h — web UI and HTTP endpoints
- utils/gpio.h — pin definitions for the selected board
- utils/secrets.h — Wi-Fi credentials

## Setup

Create or edit utils/secrets.h with your Wi-Fi credentials:

```c++
// ----------- secrets.h -----------//
#ifndef SECRETS_H
#define SECRETS_H

#define WIFI_SSID     "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASSWORD"

#endif
```

Then:

1. Upload the sketch to the ESP8266.
2. Open the serial monitor to see the assigned IP address.
3. Open http://<device-ip>/ in a browser.
4. Use the color picker to select a color.
5. Click “Rotate colors” to start the rainbow effect, or click it again to stop.

## Notes

- The web UI sends color values to the /setColor endpoint and mode changes to the /setMode endpoint.
- The current state can be queried through /status.
- PWM values are scaled to the ESP8266 range, from 0 to 1023.
- The last manually selected color is saved after a short period of inactivity and restored on the next boot.

## Troubleshooting

- Make sure the strip’s 5V supply and the ESP8266 share a common ground.
- Do not power the LED strip directly from the ESP8266 pins.
- If the board does not connect to Wi-Fi, verify the SSID and password in utils/secrets.h.
