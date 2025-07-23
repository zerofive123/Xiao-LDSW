#include <Arduino.h>
#include <U8x8lib.h>

  String gpsBuffer = "";
  U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);

  void parseGNGGALocation(String data) {
      // 拆分字段
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
  }

  // 转换 ddmm.mmmm 格式为十进制度
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

// the setup function runs once when you press reset or power the board
  void setup() {

    Serial.begin(9600);        // GPS 模块波特率（ATGM336H 通常为 9600）
    Serial.println("GPS Reader Ready.");
  }

  // the loop function runs over and over again forever
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

