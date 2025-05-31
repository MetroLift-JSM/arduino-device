#include <Arduino.h>

#define LED_R 14
#define LED_G 12
#define LED_B 13

// PWM 채널 정의
#define CH_R 0
#define CH_G 1
#define CH_B 2

void setup() {
  // PWM 설정: 채널, 주기, 핀 연결
  ledcSetup(CH_R, 5000, 8); ledcAttachPin(LED_R, CH_R);
  ledcSetup(CH_G, 5000, 8); ledcAttachPin(LED_G, CH_G);
  ledcSetup(CH_B, 5000, 8); ledcAttachPin(LED_B, CH_B);

  // 시작 시 OFF
  ledcWrite(CH_R, 0);
  ledcWrite(CH_G, 0);
  ledcWrite(CH_B, 0);
}

void loop() {
  // 빨강 (255, 0, 0)
  ledcWrite(CH_R, 255);
  ledcWrite(CH_G, 0);
  ledcWrite(CH_B, 0);
  delay(3000);

  // 초록 (0, 255, 0)
  ledcWrite(CH_R, 0);
  ledcWrite(CH_G, 255);
  ledcWrite(CH_B, 0);
  delay(3000);

  // 노랑 → 빨강: 255, 초록: 100 정도로 줄임
  ledcWrite(CH_R, 255);
  ledcWrite(CH_G, 100);  //초록 약하게
  ledcWrite(CH_B, 0);
  delay(3000);
}