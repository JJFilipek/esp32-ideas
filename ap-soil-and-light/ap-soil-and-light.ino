#include <WiFi.h>
#include <Wire.h>
#include <BH1750.h>
#include "veml6040.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_sleep.h"

#define SDA_PIN 8
#define SCL_PIN 9
#define SOIL_PIN 1

const char* ssid = "AP-name";
const char* password = "AP-password";
const char* server_ip = "server-ip";
const uint16_t server_port = 9000;

BH1750   bh;
VEML6040 veml;
WiFiClient client;

bool clearBus(){
  if (digitalRead(SDA_PIN)) return true;
  pinMode(SCL_PIN, OUTPUT_OPEN_DRAIN);
  for(int i=0;i<9;++i){digitalWrite(SCL_PIN,HIGH);delayMicroseconds(5);digitalWrite(SCL_PIN,LOW);delayMicroseconds(5);}
  pinMode(SCL_PIN, INPUT_PULLUP);
  return digitalRead(SDA_PIN);
}

void setup(){
  setCpuFrequencyMhz(80);
  btStop();
  Serial.begin(115200);
  while(!Serial);
  delay(3000);

  pinMode(SDA_PIN, INPUT_PULLUP);
  pinMode(SCL_PIN, INPUT_PULLUP);
  clearBus();

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  if(!bh.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) esp_deep_sleep_start();
  if(!veml.begin())                               esp_deep_sleep_start();
  veml.setConfiguration(VEML6040_IT_320MS + VEML6040_AF_AUTO + VEML6040_SD_ENABLE);

  delay(1);

  int soil = analogRead(SOIL_PIN);
  float lux = bh.readLightLevel();
  uint16_t r = veml.getRed();
  uint16_t g = veml.getGreen();
  uint16_t b = veml.getBlue();
  uint16_t w = veml.getWhite();
  float cct = veml.getCCT();

  WiFi.setTxPower(WIFI_POWER_11dBm);
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
  WiFi.begin(ssid, password);
  unsigned long t0 = millis();
  while (WiFi.status()!=WL_CONNECTED && millis()-t0<10000) delay(200);

  String json = String("{\"deviceId\":\"ESP1\",\"soil\":") + soil +
                ",\"lux\":" + lux +
                ",\"lightColor\":{\"red\":" + r +
                ",\"green\":" + g +
                ",\"blue\":" + b +
                ",\"white\":" + w +
                ",\"colorTemperature\":" + cct + "}}";

  if (WiFi.status()==WL_CONNECTED && client.connect(server_ip, server_port)) {
    client.print(json); client.stop(); Serial.println("Sent: " + json);
  } else {
    Serial.println("Failed to connect to server");
  }

  Wire.end();
  pinMode(SDA_PIN, INPUT_PULLUP);
  pinMode(SCL_PIN, INPUT_PULLUP);
  delay(1);

  esp_sleep_enable_timer_wakeup(5*60*1000000ULL);
  esp_deep_sleep_start();
}

void loop(){}
