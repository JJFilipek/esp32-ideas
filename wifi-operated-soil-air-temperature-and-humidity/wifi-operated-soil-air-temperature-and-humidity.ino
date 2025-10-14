#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <SPIFFS.h>


const char* ssid = "Network-name";
const char* password = "Network-password";

// OTA login
const char* otaUser = "admin";
const char* otaPass = "esp32update";

// Sensor pins
#define SOIL_PIN 34
#define LIGHT_PIN 33

WebServer server(80);

void setup() {
  Serial.begin(115200);

  pinMode(SOIL_PIN, INPUT);
  pinMode(LIGHT_PIN, INPUT);

  SPIFFS.begin(true);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi.");
  }

  // Enable mDNS
  if (MDNS.begin("esp32")) {
    Serial.println("mDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/data", handleData);

  // OTA update (GET form + POST upload) with authentication
  server.on("/update", HTTP_GET, []() {
    if (!server.authenticate(otaUser, otaPass)) return server.requestAuthentication();
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", updateForm());
  });

  server.on("/update", HTTP_POST, []() {
    if (!server.authenticate(otaUser, otaPass)) return server.requestAuthentication();
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain",
                (Update.hasError()) ? "Firmware Update Failed!" : "Firmware Update Successful!");
    ESP.restart();
  }, handleUpdateUpload);

  server.begin();
  Serial.println("HTTP server started!");
}

void loop() {
  server.handleClient();

  // Auto reconnect Wi-Fi if disconnected
  static unsigned long lastReconnectAttempt = 0;
  if (WiFi.status() != WL_CONNECTED && millis() - lastReconnectAttempt > 5000) {
    Serial.println("WiFi lost — reconnecting...");
    WiFi.reconnect();
    lastReconnectAttempt = millis();
  }
}

// Main page
void handleRoot() {
  File file = SPIFFS.open("/index.html", "r");
  if (!file) {
    server.send(500, "text/plain", "index.html not found");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}


// JSON data
void handleData() {
  int soilRaw = analogRead(SOIL_PIN);
  int lightRaw = analogRead(LIGHT_PIN);

  float soilPercent = constrain((1.0 - soilRaw / 4095.0) * 100.0, 0.0, 100.0);
  float lightPercent = constrain((1.0 - lightRaw / 4095.0) * 100.0, 0.0, 100.0);

  char json[100];
  snprintf(json, sizeof(json),
           "{\"soilRaw\":%d,\"soilPercent\":%.1f,\"lightRaw\":%d,\"lightPercent\":%.1f}",
           soilRaw, soilPercent, lightRaw, lightPercent);

  server.send(200, "application/json", json);
}

// OTA form
String updateForm() {
  return "<form method='POST' action='/update' enctype='multipart/form-data'>"
         "<input type='file' name='update'>"
         "<input type='submit' value='Upload'>"
         "</form>";
}

// OTA upload
void handleUpdateUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Updating: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
      Update.printError(Serial);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) Serial.printf("Update Success: %u bytes\n", upload.totalSize);
    else Update.printError(Serial);
  }
}
