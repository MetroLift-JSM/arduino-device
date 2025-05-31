#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <math.h>

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
}

void loop() {
    if (millis() - lastScanTime >= 3000) {
        Serial.println("[스캔 시작]");
        int sampleCount = 3;
        int rssiTotal = 0;
        int validCount = 0;
        int txPower = -59;
        double distance = 999;

    for (int s = 0; s < sampleCount; s++) {
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

            if (strcasecmp(uuid, targetUUID) == 0 &&
                major == targetMajor &&
                minor == targetMinor) {
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
            Serial.println("YELLOW LED ON (기준 만족: 5미터 이내 거리 또는 신호 강함)");
        } else if ((distance <= 20.0 && distance > 10.0) || (avgRSSI <= -65 && avgRSSI > -75)) {
            setLEDColor(0, 255, 0);    // GREEN
            Serial.println("GREEN LED ON (기준 만족: 10미터 이내 거리 또는 신호 보통)");
        } else {
            setLEDColor(0, 0, 0);      // OFF
            Serial.println("LED OFF (비콘 없음 또는 거리/신호 약함)");
        }
        } else {
        setLEDColor(0, 0, 0);        // OFF
        Serial.println("LED OFF (비콘 없음 또는 거리/신호 약함)");
    }

        lastScanTime = millis();
    }
}