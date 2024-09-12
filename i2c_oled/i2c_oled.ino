/* I2C接続のOLED を駆動します。

OLEDは SSD1306、モノクロ表示、128x64ドットのものを、PB8(SCL), PB9(SDA)ピンを利用して接続してください。
動作が安定しない場合、外部プルアップをしてみてください。

外部ライブラリが必要です。Arduinoのライブラリマネージャーからインストールしてください。
 - Adafruit SSD1306 by Adafruit

このライブラリはCH32Vに正式には対応していないため、ファイルのインクルードでエラーが発生します。
エラーを抑止するために、util/delay.h というダミーのファイルを配置し、読み込ませています。

*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// 商品によってI2Cアドレスが異なる場合があります。必要に応じて修正してください
#define OLED_ADDR (0x78 >> 1)
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);

  // Suzuno32RVのI2Cピン割り当てに合わせて、ピン番号を変更します
  Wire.setSCL(PB8);
  Wire.setSDA(PB9);
  Wire.begin();

  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 allocation failed.");
    while (true) { }
  }

  oled.display();
  oled.clearDisplay();
  oled.display();
}

void loop() {

  oled.clearDisplay();

  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(8, 16);
  oled.setTextSize(2);
  oled.println("Suzuno32RV");
  oled.setCursor(16, 48);
  oled.setTextSize(1);
  oled.println("  OLED example");
  oled.display();

  delay(1000);

  draw_randompixels();
}

void draw_randompixels() {
  static int seed = 0;
  randomSeed(seed++);
  for (size_t i = 0; i < 300; i++) {
    int x = random(0, OLED_WIDTH - 1);
    int y = random(0, OLED_HEIGHT - 1);
    oled.fillRect(x, y, 3, 3, SSD1306_WHITE);
    oled.display();
  }
}
