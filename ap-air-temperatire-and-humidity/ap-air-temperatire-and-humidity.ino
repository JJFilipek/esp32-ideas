#include <WiFi.h>
#include <Wire.h>
#include "Adafruit_SHTC3.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_sleep.h"

#define SDA_PIN 8
#define SCL_PIN 9

const char* ssid = "AP-name";
const char* password = "AP-password";
const char* server_ip = "server-ip";
const uint16_t server_port = 9000;

Adafruit_SHTC3 shtc3; WiFiClient client;

bool clearBus(){
  if (digitalRead(SDA_PIN)) return true;
  pinMode(SCL_PIN,OUTPUT_OPEN_DRAIN);
  for(int i=0;i<9;++i){digitalWrite(SCL_PIN,HIGH);delayMicroseconds(5);digitalWrite(SCL_PIN,LOW);delayMicroseconds(5);}
  pinMode(SCL_PIN,INPUT_PULLUP);
  return digitalRead(SDA_PIN);
}

void setup(){
  setCpuFrequencyMhz(80);
  btStop();
  Serial.begin(115200);
  while(!Serial);
  delay(1000);

  pinMode(SDA_PIN,INPUT_PULLUP);
  pinMode(SCL_PIN,INPUT_PULLUP);
  clearBus();

  Wire.begin(SDA_PIN,SCL_PIN);
  Wire.setClock(100000);

  if(!shtc3.begin()) esp_deep_sleep_start();
  shtc3.reset();
  delay(1);

  sensors_event_t h,t;
  if(!shtc3.getEvent(&h,&t)) esp_deep_sleep_start();

  WiFi.setTxPower(WIFI_POWER_11dBm);
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
  WiFi.begin(ssid,password);
  unsigned long t0=millis();
  while(WiFi.status()!=WL_CONNECTED&&millis()-t0<10000) delay(200);

  String payload=String("{\"deviceId\":\"ESP2\",\"temperature\":")+t.temperature+
                 ",\"humidity\":"+h.relative_humidity+"}";

  if(WiFi.status()==WL_CONNECTED&&client.connect(server_ip,server_port)){
    client.print(payload); client.stop(); Serial.println("Sent: "+payload);
  }else Serial.println("Failed to connect to server");

  Wire.end();
  pinMode(SDA_PIN, INPUT_PULLUP);
  pinMode(SCL_PIN, INPUT_PULLUP);
  delay(1);

  esp_sleep_enable_timer_wakeup(5*60*1000000ULL);
  esp_deep_sleep_start();
}

void loop(){}
