#include <WiFi.h>
#include <HTTPClient.h>

// Wi-Fi credentials
const char* ssid = "MERCUSYS_246";
const char* password = "01056210054";

// Server URL
const char* serverUrl = "http://192.168.1.3:3000/";

void setup() {
  Serial.begin(115200);
  delay(1000);  // 시리얼 모니터 준비 시간 확보

  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  Serial.println("Starting HTTP request");

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  String jsonData = "{\"message\":\"Data sent from ESP32\"}";
  Serial.println("Sending JSON: " + jsonData);

  int httpCode = http.POST(jsonData);

  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("HTTP response code: " + String(httpCode));
    Serial.println("Server response: " + response);
  } else {
    Serial.println("HTTP request failed: " + String(http.errorToString(httpCode)));
  }

  http.end();
  Serial.println("HTTP request finished");
}

void loop() {
  // Do nothing
}