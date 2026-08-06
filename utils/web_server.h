#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESP8266WebServer.h>
#include "led_control.h"
#include "persistence.h"

static ESP8266WebServer server(80);

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
  button {
    margin-top:24px; padding:12px 28px; font-size:15px; border-radius:24px;
    border:1px solid #444; background:#222; color:#eee; cursor:pointer;
  }
  button.active { background:#eee; color:#111; }
  .status { margin-top:20px; opacity:0.7; font-size:14px; }
</style>
</head>
<body>
  <h1>LED Strip Control</h1>
  <input type="color" id="colorPicker" value="#ff0000">
  <br>
  <button id="rotateBtn">Rotate colors</button>
  <p class="status" id="status">Connected</p>

<script>
const picker = document.getElementById('colorPicker');
const rotateBtn = document.getElementById('rotateBtn');
const status = document.getElementById('status');
let debounceTimer;
let rotating = false;

picker.addEventListener('input', () => {
  clearTimeout(debounceTimer);
  debounceTimer = setTimeout(sendColor, 80);
});

rotateBtn.addEventListener('click', () => {
  setMode(rotating ? 'static' : 'rotate');
});

function sendColor() {
  const hex = picker.value;
  const r = parseInt(hex.substr(1, 2), 16);
  const g = parseInt(hex.substr(3, 2), 16);
  const b = parseInt(hex.substr(5, 2), 16);
  fetch(`/setColor?r=${r}&g=${g}&b=${b}`)
    .then(() => { setRotatingUI(false); status.textContent = 'Updated'; })
    .catch(() => status.textContent = 'Connection error');
}

function setMode(mode) {
  fetch(`/setMode?mode=${mode}`)
    .then(() => { setRotatingUI(mode === 'rotate'); status.textContent = 'Updated'; })
    .catch(() => status.textContent = 'Connection error');
}

function setRotatingUI(isRotating) {
  rotating = isRotating;
  rotateBtn.textContent = rotating ? 'Stop rotation' : 'Rotate colors';
  rotateBtn.classList.toggle('active', rotating);
  picker.disabled = rotating;
}

// Reflect the board's current state when the page loads
fetch('/status')
  .then(res => res.json())
  .then(s => {
    const hex = '#' + [s.r, s.g, s.b].map(v => v.toString(16).padStart(2, '0')).join('');
    picker.value = hex;
    setRotatingUI(s.mode === 'rotate');
  });
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

    ledSetMode(MODE_STATIC); // picking a color manually stops any rotation
    ledSetColor(r, g, b);
    scheduleColorSave();

    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing r, g or b parameter");
  }
}

inline void handleSetMode() {
  if (!server.hasArg("mode")) {
    server.send(400, "text/plain", "Missing mode parameter");
    return;
  }

  String mode = server.arg("mode");
  if (mode == "rotate") {
    ledSetMode(MODE_ROTATE);
  } else if (mode == "static") {
    ledSetMode(MODE_STATIC);
  } else {
    server.send(400, "text/plain", "Invalid mode, use 'rotate' or 'static'");
    return;
  }

  server.send(200, "text/plain", "OK");
}

inline void handleStatus() {
  RGBColor c = ledGetColor();
  String mode = (ledGetMode() == MODE_ROTATE) ? "rotate" : "static";
  String json = "{\"r\":" + String(c.r) + ",\"g\":" + String(c.g) +
                ",\"b\":" + String(c.b) + ",\"mode\":\"" + mode + "\"}";
  server.send(200, "application/json", json);
}

inline void webServerInit() {
  server.on("/", handleRoot);
  server.on("/setColor", handleSetColor);
  server.on("/setMode", handleSetMode);
  server.on("/status", handleStatus);
  server.begin();
}

inline void webServerLoop() {
  server.handleClient();
}

#endif