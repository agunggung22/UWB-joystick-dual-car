#include <SPI.h>
#include "DW1000Jang.hpp"
#include "DW1000JangUtils.hpp"
#include "DW1000JangTime.hpp"
#include "DW1000JangConstants.hpp"

// 조이스틱과 버튼 핀 설정
const int VRX_PIN = A0;
const int VRY_PIN = A1;
const int BUTTON_PIN = A2;

const int SW1_PIN = A3;     // MAC: 수신기 6번 선택
const int SW2_PIN = A4;     // MAC: 수신기 7번 선택

// 데이터 전송용 버퍼
#define LEN_DATA 128
byte data[LEN_DATA];

// 방향 정의
enum Direction {
  STOP = 0,
  NORTH,
  SOUTH,
  WEST,
  EAST
};

// UWB 기본 설정
device_configuration_t DEFAULT_CONFIG = {
  false, true, true, true, false,
  SFDMode::STANDARD_SFD,
  Channel::CHANNEL_7, // 5로 하면 다른 조종기의 영향 받음
  DataRate::RATE_850KBPS,
  PulseFrequency::FREQ_64MHZ,
  PreambleLength::LEN_2048,
  PreambleCode::CODE_10
};

interrupt_configuration_t DEFAULT_INTERRUPT_CONFIG = {
  false, true, true, false, true
};

void setup() {
 

  pinMode(VRX_PIN, INPUT);
  pinMode(VRY_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(SW1_PIN, INPUT_PULLUP);
  pinMode(SW2_PIN, INPUT_PULLUP);


  DW1000Jang::initialize(10, 2, 7);
  DW1000Jang::applyConfiguration(DEFAULT_CONFIG);
  DW1000Jang::applyInterruptConfiguration(DEFAULT_INTERRUPT_CONFIG);

  DW1000Jang::setNetworkId(10);
  DW1000Jang::setDeviceAddress(5);
  DW1000Jang::setAntennaDelay(16436);

  Serial.println(F("Ready to send joystick commands..."));
}

void loop() {
  Direction dir;
  int speed;
  getDirectionAndSpeed(dir, speed);

  data[0] = 0x01;
  data[1] = (byte)dir;
  data[2] = speed; // 방향별 계산된 속도

  // 스위치 입력 읽기
  bool sw1 = (digitalRead(SW1_PIN) == LOW);
  bool sw2 = (digitalRead(SW2_PIN) == LOW);
   if (!sw1 && !sw2) {
    Serial.println("[No Switch Pressed] Not sending.");
    return;
  }

  // 목적지 설정 및 전송
  if (sw1) {
    data[3] = 6;
    DW1000Jang::setTransmitData(data, 4);
    DW1000Jang::startTransmit(TransmitMode::IMMEDIATE);
    Serial.println("[Sent to 6]");
    delay(100); // ← 간단한 전송 간 딜레이
  }
  if (sw2) {
    data[3] = 7;
    DW1000Jang::setTransmitData(data, 4);
    DW1000Jang::startTransmit(TransmitMode::IMMEDIATE);
    Serial.println("[Sent to 7]");
    delay(100); // ← 간단한 전송 간 딜레이
  }

  delay(100); // 빠른 반응성

  // 추가: 조이스틱 값 출력
  int x = analogRead(VRX_PIN);
  int y = analogRead(VRY_PIN);
  Serial.print("X = ");
  Serial.print(x);
  Serial.print(", Y = ");
  Serial.println(y);

  // int buttonPressed = !digitalRead(BUTTON_PIN);
  // if (buttonPressed) {
  //   Serial.println("1!");//1
  // } else {
  //   Serial.println("0!");//0
  // }
}

// 방향과 속도 함께 계산하는 함수
void getDirectionAndSpeed(Direction &dir, int &speed) {
  int x = analogRead(VRX_PIN);
  int y = analogRead(VRY_PIN);
  int buttonPressed = !digitalRead(BUTTON_PIN);

  if (buttonPressed) {
    dir = STOP;
    speed = 0;
    return;
  }



  // 방향 및 속도 결정
  if (y > 570) { // SOUTH
    dir = WEST;
    speed = map(y, 570, 940, 0, 100); // y 570~940 → 속도 0~100
    // speed=speed-speed%10;
    speed = constrain(speed, 0, 100); // 0보다 작으면 0, 크면 100을 반환해서 범위를 제한
  }
  else if (y < 370) { // NORTH
    dir = EAST;
    speed = map(y, 0, 370, 100, 0);  // y 0~370 → 속도 100~0
    // speed=speed-speed%10;
    speed = constrain(speed, 0, 100);
  }
  else if (x > 800) { // EAST
    dir = SOUTH;
    speed = map(x, 800, 920, 0, 100); // x 800~920 → 속도 0~100
    // speed=speed-speed%10;
    speed = constrain(speed, 0, 100);
  }
  else if (x < 700) { // WEST
    dir = NORTH;
    speed = map(x, 220, 700, 100, 0); // x 220~700 → 속도 100~0
    // speed=speed-speed%10;
    speed = constrain(speed, 0, 100);
  }
  else {
    dir = STOP;
    speed = 0;
  }
}
