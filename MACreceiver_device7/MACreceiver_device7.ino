
#include <SPI.h>
#include "DW1000Jang.hpp"
#include "DW1000JangUtils.hpp"
#include "DW1000JangTime.hpp"
#include "DW1000JangConstants.hpp"

// 모터 핀 설정
#define LEFT_FRONT_PWM 5
#define LEFT_FRONT_DIR 4
#define RIGHT_FRONT_PWM 6
#define RIGHT_FRONT_DIR A1
#define RIGHT_BACK_PWM 9
#define RIGHT_BACK_DIR 8
#define LEFT_BACK_PWM 3
#define LEFT_BACK_DIR A2

// 수신 데이터 버퍼
#define LEN_DATA 128
    byte data[LEN_DATA];

// 내 MAC 주소 선언
const byte MY_ADDRESS = 7;

// 방향 정의
enum Direction
{
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
    Channel::CHANNEL_7,
    DataRate::RATE_850KBPS,
    PulseFrequency::FREQ_64MHZ,
    PreambleLength::LEN_2048,
    PreambleCode::CODE_10};

interrupt_configuration_t DEFAULT_INTERRUPT_CONFIG = {
    false, true, true, false, true};

void setup()
{
  
  pinMode(LEFT_FRONT_PWM, OUTPUT);
  pinMode(LEFT_FRONT_DIR, OUTPUT);
  pinMode(RIGHT_FRONT_PWM, OUTPUT);
  pinMode(RIGHT_FRONT_DIR, OUTPUT);
  pinMode(RIGHT_BACK_PWM, OUTPUT);
  pinMode(RIGHT_BACK_DIR, OUTPUT);
  pinMode(LEFT_BACK_PWM, OUTPUT);
  pinMode(LEFT_BACK_DIR, OUTPUT);

  DW1000Jang::initialize(10, 2, 7);
  DW1000Jang::applyConfiguration(DEFAULT_CONFIG);
  DW1000Jang::applyInterruptConfiguration(DEFAULT_INTERRUPT_CONFIG);

  DW1000Jang::setNetworkId(10);
  DW1000Jang::setDeviceAddress(MY_ADDRESS);
  DW1000Jang::setAntennaDelay(16436);

  DW1000Jang::startReceive();
}

void loop()
{
  if (DW1000Jang::isReceiveDone())
  {                                              // 수신 완료되었을 때만
    DW1000Jang::getReceivedData(data, LEN_DATA); // 데이터 읽기

    if (data[0] != 0x01 || data[3] != MY_ADDRESS)
    {
      Serial.println("Not for me or invalid data");
      DW1000Jang::clearReceiveStatus();
      DW1000Jang::startReceive();
      return;
    }

    
      Serial.println("[Received Raw Data]");
      for (int i = 0; i < 3; i++)
      {
        Serial.print("data[");
        Serial.print(i);
        Serial.print("]: ");
        Serial.println(data[i]);
      }

      int received_direction = (int)data[1];
      int received_speed = (int)data[2];

      controlMotors((Direction)received_direction, received_speed);
    

    DW1000Jang::clearReceiveStatus(); // 상태 클리어
    DW1000Jang::startReceive();       // 다음 수신 대기
  }
}

void controlMotors(Direction dir, int speed)
{
  switch (dir)
  {
  case NORTH:
    moveForward(speed);
    break;
  case SOUTH:
    moveBackward(speed);
    break;
  case WEST:
    moveLeft(speed);
    break;
  case EAST:
    moveRight(speed);
    break;
  case STOP:
  default:
    Stop();
    break;
  }
}

void moveForward(int speed)
{
  analogWrite(LEFT_FRONT_PWM, speed);
  digitalWrite(LEFT_FRONT_DIR, HIGH);
  analogWrite(RIGHT_FRONT_PWM, speed);
  digitalWrite(RIGHT_FRONT_DIR, LOW);
  analogWrite(RIGHT_BACK_PWM, speed);
  digitalWrite(RIGHT_BACK_DIR, HIGH);
  analogWrite(LEFT_BACK_PWM, speed);
  digitalWrite(LEFT_BACK_DIR, LOW);
}

void moveBackward(int speed)
{
  analogWrite(LEFT_FRONT_PWM, speed);
  digitalWrite(LEFT_FRONT_DIR, LOW);
  analogWrite(RIGHT_FRONT_PWM, speed);
  digitalWrite(RIGHT_FRONT_DIR, HIGH);
  analogWrite(RIGHT_BACK_PWM, speed);
  digitalWrite(RIGHT_BACK_DIR, LOW);
  analogWrite(LEFT_BACK_PWM, speed);
  digitalWrite(LEFT_BACK_DIR, HIGH);
}

void moveLeft(int speed)
{
  analogWrite(LEFT_FRONT_PWM, speed);
  digitalWrite(LEFT_FRONT_DIR, LOW);
  analogWrite(RIGHT_FRONT_PWM, speed);
  digitalWrite(RIGHT_FRONT_DIR, LOW);
  analogWrite(RIGHT_BACK_PWM, speed);
  digitalWrite(RIGHT_BACK_DIR, HIGH);
  analogWrite(LEFT_BACK_PWM, speed);
  digitalWrite(LEFT_BACK_DIR, HIGH);
}

void moveRight(int speed)
{
  analogWrite(LEFT_FRONT_PWM, speed);
  digitalWrite(LEFT_FRONT_DIR, HIGH);
  analogWrite(RIGHT_FRONT_PWM, speed);
  digitalWrite(RIGHT_FRONT_DIR, HIGH);
  analogWrite(RIGHT_BACK_PWM, speed);
  digitalWrite(RIGHT_BACK_DIR, LOW);
  analogWrite(LEFT_BACK_PWM, speed);
  digitalWrite(LEFT_BACK_DIR, LOW);
}

void Stop()
{
  analogWrite(LEFT_FRONT_PWM, 0);
  analogWrite(RIGHT_FRONT_PWM, 0);
  analogWrite(RIGHT_BACK_PWM, 0);
  analogWrite(LEFT_BACK_PWM, 0);
}
