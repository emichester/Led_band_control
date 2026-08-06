#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESP8266WebServer.h>
#include "led_control.h"

static ESP8266WebServer server(80);

// Web UI: a single page with a color picker. On change, it sends the new
// color to the board via a fetch() call, debounced so it doesn't flood
// the ESP8266 while the user is dragging the picker.
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>LED Strip Control</title>
<style>
  body { font-family: sans-serif; text-align: center; background:#111; color:#eee; padding-top:40px; }
  input[type=color] { width:200px; height:200px; border:none; border-radius:16px; cursor:pointer; }
  h1 { font-weight:300; }
  .status { margin-top:20px; opacity:0.7; font-size:14px; }
</style>
</head>
<body>
  <h1>LED Strip Control</h1>
  <input type="color" id="colorPicker" value="#ff0000">
  <p class="status" id="status">Connected</p>

<script>
const picker = document.getElementById('colorPicker');
const status = document.getElementById('status');
let debounceTimer;

picker.addEventListener('input', () => {
  clearTimeout(debounceTimer);
  debounceTimer = setTimeout(sendColor, 80);
});

function sendColor() {
  const hex = picker.value;
  const r = parseInt(hex.substr(1, 2), 16);
  const g = parseInt(hex.substr(3, 2), 16);
  const b = parseInt(hex.substr(5, 2), 16);
  fetch(`/setColor?r=${r}&g=${g}&b=${b}`)
    .then(() => status.textContent = 'Updated')
    .catch(() => status.textContent = 'Connection error');
}
</script>
</body>
</html>
)rawliteral";

inline void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

inline void handleSetColor() {
  if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
    uint8_t r = server.arg("r").toInt();
    uint8_t g = server.arg("g").toInt();
    uint8_t b = server.arg("b").toInt();
    ledSetColor(r, g, b);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing r, g or b parameter");
  }
}

inline void handleStatus() {
  RGBColor c = ledGetColor();
  String json = "{\"r\":" + String(c.r) + ",\"g\":" + String(c.g) + ",\"b\":" + String(c.b) + "}";
  server.send(200, "application/json", json);
}

inline void webServerInit() {
  server.on("/", handleRoot);
  server.on("/setColor", handleSetColor);
  server.on("/status", handleStatus);
  server.begin();
}

inline void webServerLoop() {
  server.handleClient();
}

#endif