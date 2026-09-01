#ifndef NETWORK_H
#define NETWORK_H

#include <ESP8266WiFi.h>
#include "secrets.h"

// Starts the D1 mini as its own WiFi access point instead of joining an
// existing router. The Android app connects directly to this network to
// control the strip, with no home WiFi or internet required.
inline void startAccessPoint() {
  Serial.print("Starting access point: ");
  Serial.println(AP_SSID);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  Serial.print("Access point ready. IP address: ");
  Serial.println(WiFi.softAPIP()); // usually 192.168.4.1
}

#endif