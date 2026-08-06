#ifndef NETWORK_H
#define NETWORK_H

#include <ESP8266WiFi.h>
#include "secrets.h"

// How often to retry a reconnect attempt while WiFi is down (ms)
#define WIFI_RECONNECT_INTERVAL 5000

static unsigned long lastReconnectAttempt = 0;

// Blocking connect, meant to be called once from setup().
inline void connectToWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
}

// Non-blocking watchdog, meant to be called on every loop() iteration.
// If the connection drops, it retries every WIFI_RECONNECT_INTERVAL ms
// without freezing the rest of the program (e.g. the web server keeps
// responding to already-connected clients in the meantime).
inline void maintainWiFiConnection() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  unsigned long now = millis();
  if (now - lastReconnectAttempt < WIFI_RECONNECT_INTERVAL) {
    return;
  }
  lastReconnectAttempt = now;

  Serial.println("WiFi disconnected, attempting to reconnect...");
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

#endif