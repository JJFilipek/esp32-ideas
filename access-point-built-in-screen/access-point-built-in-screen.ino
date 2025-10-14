#include <WiFi.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "esp_wifi.h"
#include "esp_system.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6, 5);

const char *ssid = "AP-Name";
const char *password = "AP-Password";

unsigned long screenStart;

void setup() {
  setCpuFrequencyMhz(80);
  Serial.begin(115200);
  delay(500);

  btStop();

  WiFi.mode(WIFI_AP);
  WiFi.setTxPower(WIFI_POWER_15dBm); 
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

  WiFi.softAP(ssid, password);
  IPAddress ip = WiFi.softAPIP();

  u8g2.begin();
  u8g2.setContrast(255);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.drawStr(26, 32, ssid);
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(26, 48, ip.toString().c_str());
  u8g2.sendBuffer();

  Serial.println("Access Point started");
  Serial.print("SSID: "); Serial.println(ssid);
  Serial.print("IP address: "); Serial.println(ip);

  Serial.end(); 
  screenStart = millis();
}

void loop() {
  if (millis() - screenStart > 10000) {
    u8g2.setPowerSave(1);
  }
  delay(1000);
}
