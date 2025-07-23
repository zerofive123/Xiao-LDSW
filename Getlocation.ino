#include <Arduino.h>
#include <U8x8lib.h>

// OLED 初始化（使用硬件 I2C）
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);

String gpsBuffer = "";

void setup() {
  Serial.begin(9600);
  u8x8.begin();               // 启动 OLED
  u8x8.setFlipMode(1);        // 显示方向反转（如需）
  u8x8.setFont(u8x8_font_chroma48medium8_r);  // 设置字体
  u8x8.drawString(0, 0, "GPS Ready...");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      gpsBuffer.trim();
      if (gpsBuffer.startsWith("$GNGGA")) {
        parseGNGGALocation(gpsBuffer);
      }
      gpsBuffer = "";
    } else {
      gpsBuffer += c;
    }
  }
}

void parseGNGGALocation(String data) {
  int commaIndex = 0;
  int fieldNum = 0;
  String fields[15];

  for (int i = 0; i < data.length(); i++) {
    if (data.charAt(i) == ',' || data.charAt(i) == '*') {
      fields[fieldNum++] = data.substring(commaIndex, i);
      commaIndex = i + 1;
    }
  }

  String rawLat = fields[2];
  String latDir = fields[3];
  String rawLon = fields[4];
  String lonDir = fields[5];

  String latitude = convertToDecimal(rawLat, latDir);
  String longitude = convertToDecimal(rawLon, lonDir);

  Serial.println("Latitude : " + latitude);
  Serial.println("Longitude: " + longitude);
  Serial.println("-------------------------");

  // 显示到 OLED（建议只保留小数点后4位，避免溢出）
  u8x8.clear();
  u8x8.drawString(0, 0, "GPS Location:");
  u8x8.drawString(0, 2, "Lat:");
  u8x8.drawString(4, 2, latitude.substring(0, 10).c_str());
  u8x8.drawString(0, 4, "Lon:");
  u8x8.drawString(4, 4, longitude.substring(0, 10).c_str());
}

// 经纬度格式转换
String convertToDecimal(String raw, String dir) {
  if (raw.length() < 6) return "Invalid";

  int dotIndex = raw.indexOf('.');
  int degLength = (dotIndex == 4) ? 2 : 3;

  float degrees = raw.substring(0, degLength).toFloat();
  float minutes = raw.substring(degLength).toFloat();
  float decimal = degrees + (minutes / 60.0);

  if (dir == "S" || dir == "W") decimal *= -1;

  return String(decimal, 6);
}
