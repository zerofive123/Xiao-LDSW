#include <Arduino.h>

const int wakePin = D1;       // WAKE_IN引脚
const int modePin = D2;       // MODE_SW引脚
#define ldswSerial Serial1    // 硬件串口1固定为 P1.13(TX)/P1.12(RX)

// CRC16计算（Modbus标准）
uint16_t calculateCRC(const byte *data, uint16_t len) {
  uint16_t crc = 0xFFFF;
  while (len--) {
    crc ^= *data++;
    for (uint8_t i = 0; i < 8; i++) {
      if (crc & 1) crc = (crc >> 1) ^ 0xA001;
      else crc >>= 1;
    }
  }
  return crc;
}

void sendCommand(const byte* cmd, uint16_t len) {
  digitalWrite(wakePin, LOW);  // 确保模块完全唤醒
  delay(20);                   // 文档要求至少5ms
  ldswSerial.write(cmd, len);
  ldswSerial.flush();
  digitalWrite(wakePin, HIGH); // 释放WAKE_IN
}

void setup() {
  pinMode(wakePin, OUTPUT);
  pinMode(modePin, OUTPUT);
  digitalWrite(modePin, LOW);  // 强制进入配置模式（LOW电平）

  ldswSerial.begin(9600);     // 初始化LDSW硬件串口
  Serial.begin(115200);       // USB调试输出
  while (!Serial);            // 等待串口就绪
  Serial.println("LDSW已强制进入配置模式（MODE_SW= LOW）");
}

void loop() {
  byte queryCmd[] = {
    0xA5, 0x5A,       // 帧头（小端）
    0x10, 0x00,       // 长度=16字节
    0x01,             // 帧版本
    0x11, 0x11,       // 事件号
    0x01, 0x00,       // 命令码0x0001（参数读取）
    0x01, 0x00, 0x20, 0x41, // 参数Code 0x41200001
    0x01,              // 操作命令（读取）

  };

  sendCommand(queryCmd, sizeof(queryCmd));

  // 接收响应（带超时）
  uint32_t start = millis();
  byte response[32];
  int len = 0;
  
  while (millis() - start < 500) {
    if (ldswSerial.available()) {
      response[len] = ldswSerial.read();
      len++;
      if (len >= 16) break; // 最小响应长度16字节
    }
    delay(1);
  }

  // 打印结果
  if (len > 0) {
    Serial.print("响应HEX: ");
    for (int i = 0; i < len; i++) {
      if (response[i] < 0x10) Serial.print("0");
      Serial.print(response[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  } else {
    Serial.println("无响应！请检查：");
    Serial.println("1. MODE_SW是否保持LOW电平");
    Serial.println("2. 串口接线是否正确（TX/RX交叉连接）");
  }
  delay(2000);
}