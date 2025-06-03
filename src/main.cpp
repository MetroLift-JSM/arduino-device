#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <MFRC522.h>
#include <SPI.h>
#include <math.h>
#include <WiFi.h>
#include <HTTPClient.h>

// BLE 설정
BLEScan* pBLEScan;
const char* targetUUID = "e2c56db5-dffb-48d2-b060-d0f5a71096e0";
const uint16_t targetMajor = 40011;
const uint16_t targetMinor = 52573;
unsigned long lastScanTime = 0;

// LED 핀 및 채널 설정
#define LED_R 14
#define LED_G 12
#define LED_B 13
#define CH_R 0
#define CH_G 1
#define CH_B 2

// NFC 설정
#define RST_PIN 27
#define SS_PIN 5
MFRC522 rfid(SS_PIN, RST_PIN);

// 상태 관리
bool isRed = false;
byte lastUID[4] = {0};
unsigned long redStartTime = 0;

// Wi-Fi 설정
const char* ssid = "MERCUSYS_246";
const char* password = "01056210054";

// 서버 주소
const char* serverURL = "http://192.168.1.3:3000/";

// LED 색상 설정
void setLEDColor(int r, int g, int b) {
    ledcWrite(CH_R, r); 
    ledcWrite(CH_G, g);
    ledcWrite(CH_B, b);
}

// 거리 계산 함수
double calculateDistance(int txPower, int rssi) {
    double n = 2.7;
    return pow(10.0, ((txPower - rssi) / (10.0 * n)));
}

// UID 비교
bool compareUID(byte *uid1, byte *uid2) {
  for (int i = 0; i < 4; i++) {
    if (uid1[i] != uid2[i]) return false;
  }
  return true;
}

// 서버에 상태 전송
void sendStatus(String status) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    String json = "{\"message\":\"" + status + "\"}";
    int httpCode = http.POST(json);
    String response = http.getString();
    
    Serial.println("서버 응답 코드: " + String(httpCode));
    Serial.println("서버 응답 내용: " + response);
    http.end();
  } else {
    Serial.println("WiFi 연결 안됨");
  }
}

void setup() {
    Serial.begin(115200);
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);

    // RGB LED 설정
    ledcSetup(CH_R, 5000, 8); ledcAttachPin(LED_R, CH_R);
    ledcSetup(CH_G, 5000, 8); ledcAttachPin(LED_G, CH_G);
    ledcSetup(CH_B, 5000, 8); ledcAttachPin(LED_B, CH_B);
    setLEDColor(0, 0, 0);  // 시작 시 OFF

    SPI.begin();
    rfid.PCD_Init();

  // NFC 설정
    SPI.begin();
    rfid.PCD_Init();

    // Wi-Fi 연결
    WiFi.begin(ssid, password);
    Serial.print("WiFi 연결 중");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWiFi 연결 완료");
    Serial.println("시작 완료");
  }

void loop() {
  // NFC 태깅 처리
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    byte* currentUID = rfid.uid.uidByte;

    if (!isRed) {
      // 첫 태깅 -> RED ON
      memcpy(lastUID, currentUID, 4);
      setLEDColor(255, 0, 0);
      sendStatus("RED");
      Serial.println("RED LED ON (탑승 인식)");
      isRed = true;
      redStartTime = millis();
    } else {
      // 이미 탑승 중 -> UID 비교
      if (!compareUID(currentUID, lastUID)) {
        // 다른 UID -> 하차
        Serial.println("✅ 하차 인식 - RED OFF");
        isRed = false;
      } else {
        Serial.println("같은 UID 재태깅 -> 무시");
      }
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    delay(500);
  }

    // 5분 경과 -> RED 자동 해제
    if (isRed && millis() - redStartTime >= 300000) {
      Serial.println("5분 경과 -> RED OFF");
      isRed = false;
    }

    // RED 상태면 거리 감지 생략
    if (isRed) return;

    // BLE 거리 측정 (3초마다)
    if (millis() - lastScanTime >= 3000) {
        Serial.println("[스캔 시작]");
        int rssiTotal = 0;
        int validCount = 0;
        int txPower = -59;
        double distance = 999;

    for (int s = 0; s < 3; s++) {
      BLEScanResults results = pBLEScan->start(1, false);
        for (int i = 0; i < results.getCount(); i++) {
          BLEAdvertisedDevice device = results.getDevice(i);
          std::string payload = device.getManufacturerData();
          if (payload.length() >= 25) {
            const uint8_t* data = (const uint8_t*)payload.data();
            // UUID 추출
            char uuid[37];
            sprintf(uuid,
              "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
              data[4], data[5], data[6], data[7],
              data[8], data[9],
              data[10], data[11],
              data[12], data[13],
              data[14], data[15], data[16], data[17], data[18], data[19]);

            uint16_t major = (data[20] << 8) | data[21];
            uint16_t minor = (data[22] << 8) | data[23];
            int rssi = device.getRSSI();

            if (strcasecmp(uuid, targetUUID) == 0 && major == targetMajor && minor == targetMinor) {
                rssiTotal += rssi;
                validCount++;
            }
          }
        }
      pBLEScan->clearResults();
      delay(100);
    }

    if (validCount > 0) {
      int avgRSSI = rssiTotal / validCount;
      distance = calculateDistance(txPower, avgRSSI);
      Serial.printf("평균 RSSI: %d | 추정 거리: %.2f m\n", avgRSSI, distance);

      if (distance <= 10.0 || avgRSSI > -65) {
        setLEDColor(255, 100, 0);  // YELLOW
        sendStatus("YELLOW");
        Serial.println("YELLOW LED ON (기준 만족: 5미터 이내 거리 또는 신호 강함)");
      } else {
        setLEDColor(0, 255, 0);    // GREEN
        sendStatus("GREEN");
        Serial.println("GREEN LED ON");
        }
    } else {
      // 신호가 없으면 그냥 GREEN 유지
      setLEDColor(0, 255, 0);
      sendStatus("GREEN");
      Serial.println("비콘 없음, 기본 GREEN");
    }

      lastScanTime = millis();
  }
}