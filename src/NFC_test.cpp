#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN  27                            // reset핀은 9번으로 설정
#define SS_PIN   5                           // SS핀은 10번으로 설정
                                               // SS핀은 데이터를 주고받는 역할의 핀( SS = Slave Selector )

MFRC522 rfid(SS_PIN, RST_PIN);                 // MFR522를 이용하기 위해 mfrc객체를 생성해 줍니다.

void setup(){
  Serial.begin(115200);
  while (!Serial); // 시리얼 초기화 대기 (필요할 수도 있음)
  Serial.println("SETUP 시작"); // ← 이거라도 찍혀야 정상
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("RFID 초기화 완료");
}


void loop(){
  if ( !rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial() ) {   
                                               // 태그 접촉이 되지 않았을때 또는 ID가 읽혀지지 않았을때
    delay(500);                                // 0.5초 딜레이 
    return;                                    // return
  } 
    
  Serial.print("Card UID:");                  // 태그의 ID출력
  
  for (byte i = 0; i < 4; i++) {               // 태그의 ID출력하는 반복문.태그의 ID사이즈(4)까지
    Serial.print(rfid.uid.uidByte[i]);        // mfrc.uid.uidByte[0] ~ mfrc.uid.uidByte[3]까지 출력
    Serial.print(" ");                        // id 사이의 간격 출력
  }
  Serial.println(); 
}