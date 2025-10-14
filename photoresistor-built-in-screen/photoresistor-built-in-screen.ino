#include <Wire.h>
#include <U8g2lib.h>
#include <ArduinoJson.h>

#define SENSOR_PIN 2
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6, 5);

// Sun position
const int sun_radius = 8;
const int sun_x = (128 / 2) - 4;
const int sun_y_min = 43;
const int sun_y_max = 80;

void setup() {
  Serial.begin(115200);
  delay(100);
  u8g2.begin();
  analogReadResolution(12);
  pinMode(SENSOR_PIN, INPUT);
}

void drawSunWithRays(int cx, int cy, int r) {
  // Full sun
  u8g2.drawDisc(cx, cy, r, U8G2_DRAW_ALL);

  // Rays
  int long_offset = 7;
  int short_offset = 5;
  int gap = 4;

  // NSWE – long
  u8g2.drawLine(cx, cy - r - gap - long_offset, cx, cy - r - gap); // N
  u8g2.drawLine(cx, cy + r + gap, cx, cy + r + gap + long_offset); // S
  u8g2.drawLine(cx - r - gap - long_offset, cy, cx - r - gap, cy); // W
  u8g2.drawLine(cx + r + gap, cy, cx + r + gap + long_offset, cy); // E

  // Diagonals
  u8g2.drawLine(cx - r, cy - r, cx - r - short_offset, cy - r - short_offset); // NW
  u8g2.drawLine(cx + r, cy - r, cx + r + short_offset, cy - r - short_offset); // NE
  u8g2.drawLine(cx - r, cy + r, cx - r - short_offset, cy + r + short_offset); // SW
  u8g2.drawLine(cx + r, cy + r, cx + r + short_offset, cy + r + short_offset); // SE
}

void loop() {
  static unsigned long lastPrint = 0;
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  if (now - lastUpdate >= 500) {
    lastUpdate = now;

    int raw = analogRead(SENSOR_PIN);
    int percent = map(raw, 0, 4095, 100, 0);
    int sun_y = map(percent, 0, 100, sun_y_max, sun_y_min);

    u8g2.clearBuffer();
    drawSunWithRays(sun_x, sun_y, sun_radius);
    u8g2.sendBuffer();

    if (now - lastPrint >= 2000) {
      lastPrint = now;

      StaticJsonDocument<128> doc;
      doc["timestamp"] = millis();
      doc["rawLight"] = raw;
      doc["deviceId"] = "esp32-c3-01";

      serializeJson(doc, Serial);
      Serial.println();
    }
  }
}
