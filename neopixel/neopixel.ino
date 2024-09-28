/* NeoPixel フルカラーLEDを制御します。
PA1ピンにNeoPixel信号線を接続してください。

バージョン1.12.3以降の Adafruit_Neopixel ライブラリをインストールしてください。

このコードでは、37個のNeoPixelを制御します。
*/

// NeoPixel LEDの個数
#define NEOPIXELS_COUNT 37
#define PIN_NEOPIXEL PA0


#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel pixels(NEOPIXELS_COUNT, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin();
  pixels.clear();
  pixels.show();

  for (int i = 0; i < 4; i++) {
    pixels.clear();
    pixels.show();
    delay(100);
    pixels.setPixelColor(0, 128, 128, 128);
    pixels.show();
    delay(100);
  }
}

void loop() {
  showSinglePixel();
  showAllPixels();
  showHue();
}

byte hueCurve(int x) {
  x += 1024/3;
  x = x % 1024;
  return max(255-abs(255-x*(255.0*3/1024)), 0.0);
}

uint32_t getHue(int hue) {
  byte r = hueCurve(hue);
  byte g = hueCurve(hue+(1024*2)/3);
  byte b = hueCurve(hue+1024/3);
  return r << 16 | g << 8 | b;
}


void setAllPixels(uint32_t color) {
  uint8_t r = color >> 16;
  uint8_t g = color >> 8;
  uint8_t b = color;
  for (int i = 0; i < 37; i++) {
    pixels.setPixelColor(i, r, g, b);
  }
  pixels.show();
}

void showAllPixels() {
  setAllPixels(0x101010);
  delay(500);
  setAllPixels(0x100000);
  delay(500);
  setAllPixels(0x001000);
  delay(500);
  setAllPixels(0x000010);
  delay(500);
}

void showSinglePixel() {
  for (unsigned long t = millis(); millis() - t < 1000;) {
    for (int i = 0; i < 37; i++) {
      pixels.clear();
      uint8_t r = i % 3 == 0 ? 128 : 0;
      uint8_t g = i % 3 == 1 ? 128 : 0;
      uint8_t b = i % 3 == 2 ? 128 : 0;
      pixels.setPixelColor(i, r, g, b);
      pixels.show();
      delay(80);
    }
  }
}

void showHue() {
  int cnt = 0;
  for (unsigned long t = millis(); millis() - t < 4000;) {
    for (int i = 0; i < NEOPIXELS_COUNT; i++) {
      uint32_t color = getHue(cnt);
      uint8_t r = color >> 16;
      uint8_t g = color >> 8;
      uint8_t b = color & 0xff;
      pixels.setPixelColor(i, r / 4, g / 4, b / 4);
      cnt += 1;
    }
    pixels.show();
    delay(50);
  }
}
