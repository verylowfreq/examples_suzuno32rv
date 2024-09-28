/*
LEDマトリクスモジュール HUB75e を横に2つ連結した、128x64のディスプレイを描画します。
単独での動作と、パソコンと接続してWebUSBから制御することもできます。

generate_color.py で画像データを出力できます。

ピン接続はコードを参照してください。
HUB75eモジュールは製品ごとに仕様の差異があるようなので、表示が乱れる場合は製品ごとの調整が必要かもしれません。
また、電源供給が十分でないと表示が安定しない場合もあるので、電源もご注意ください。
*/


#include "Adafruit_TinyUSB.h"

// USB WebUSB object
Adafruit_USBD_WebUSB usb_web;
WEBUSB_URL_DEF(landingPage, 1 /*https*/, "example.tinyusb.org/webusb-serial/index.html");

constexpr int PIN_LED = PA5;

void blink_led(unsigned int time_msec) {
  static bool led_on = false;
  static unsigned long timer = 0;
  if (timer == 0) {
    pinMode(PIN_LED, OUTPUT);
  }
  if (millis() - timer > time_msec) {
    led_on = !led_on;
    digitalWrite(PIN_LED, led_on ? HIGH : LOW);
    timer = millis();
  }
}

constexpr int PIN_G1 = PA15;
constexpr int PIN_B1 = PB3;
constexpr int PIN_R1 = PB4;
constexpr int PIN_G2 = PB5;
// constexpr int PIN_B2 = PD0;
// constexpr int PIN_R2 = PD1;
constexpr int PIN_B2 = PB11;
constexpr int PIN_R2 = PB10;

constexpr int PIN_CLK = PA8;
constexpr int PIN_OE = PA9;
constexpr int PIN_LAT = PA10;

constexpr int PIN_A = PA0;
constexpr int PIN_B = PA1;
constexpr int PIN_C = PA2;
constexpr int PIN_D = PA3;
constexpr int PIN_E = PA4;

constexpr int PIN_BTN_1 = PB0;
constexpr int PIN_BTN_2 = PB1;

void init_hub75e() {
  // Suzuduino UNOでのみ必要
  // RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  // GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);
  // RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

  pinMode(PIN_R1, OUTPUT);
  pinMode(PIN_G1, OUTPUT);
  pinMode(PIN_B1, OUTPUT);
  pinMode(PIN_R2, OUTPUT);
  pinMode(PIN_G2, OUTPUT);
  pinMode(PIN_B2, OUTPUT);

  pinMode(PIN_CLK, OUTPUT);
  pinMode(PIN_OE, OUTPUT);
  pinMode(PIN_LAT, OUTPUT);

  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_C, OUTPUT);
  pinMode(PIN_D, OUTPUT);
  pinMode(PIN_E, OUTPUT);
}

void select_row(uint8_t row) {
  digitalWrite(PIN_A, row & 0x01);
  digitalWrite(PIN_B, row & 0x02);
  digitalWrite(PIN_C, row & 0x04);
  digitalWrite(PIN_D, row & 0x08);
  digitalWrite(PIN_E, row & 0x10);
}

void enable_latch() {
  digitalWrite(PIN_LAT, HIGH);
}

void disable_latch() {
  digitalWrite(PIN_LAT, LOW);
}

void enable_output() {
  digitalWrite(PIN_OE, LOW);
}

void disable_output() {
  digitalWrite(PIN_OE, HIGH);
}


uint8_t vram[2][128 * 64 / 2];


void show_blank_line(uint16_t display_width) {

  digitalWrite(PIN_R1, LOW);
  digitalWrite(PIN_G1, LOW);
  digitalWrite(PIN_B1, LOW);

  digitalWrite(PIN_R2, LOW);
  digitalWrite(PIN_G2, LOW);
  digitalWrite(PIN_B2, LOW);

  for (int x = 0; x < display_width; x++) {
    digitalWrite(PIN_CLK, HIGH);
    digitalWrite(PIN_CLK, LOW);
  }

  disable_output();

  select_row(0);

  enable_latch();

  enable_output();

  disable_latch();
}

void show_bitmap(const uint8_t* bitmap, uint32_t bitmap_width, uint32_t display_width, int x_offset) {
/* 
 * Format: 4-bits per pixel ( 0, R, G, B )
 * Stride: 32-bytes (4-bits * 64-pixels)
 * Total size: 2048-bytes (32-bytes * 64-rows)
 */

  uint32_t stride = bitmap_width / 2;

  for (int y = 0; y < 32; y++) {

    for (int x = 0; x < display_width; x++) {
      uint32_t xpos = (x + x_offset) % bitmap_width;

      uint32_t byte_index_1 = stride * y + xpos / 2;
      bool is_high_nibble_1 = (xpos & 0x01) == 0;
      uint8_t pix_1 = 0;
      if (is_high_nibble_1) {
        pix_1 = bitmap[byte_index_1] >> 4;
      } else {
        pix_1 = bitmap[byte_index_1] & 0x0f;
      }
      
      uint32_t byte_index_2 = stride * (y+32) + xpos / 2;
      bool is_high_nibble_2 = (xpos & 0x01) == 0;
      uint8_t pix_2 = 0;
      if (is_high_nibble_2) {
        pix_2 = bitmap[byte_index_2] >> 4;
      } else {
        pix_2 = bitmap[byte_index_2] & 0x0f;
      }

      digitalWrite(PIN_R1, pix_1 & 0x04);
      digitalWrite(PIN_G1, pix_1 & 0x02);
      digitalWrite(PIN_B1, pix_1 & 0x01);

      digitalWrite(PIN_R2, pix_2 & 0x04);
      digitalWrite(PIN_G2, pix_2 & 0x02);
      digitalWrite(PIN_B2, pix_2 & 0x01);

      digitalWrite(PIN_CLK, HIGH);
      digitalWrite(PIN_CLK, LOW);
    }

    disable_output();

    select_row(y);

    enable_latch();

    enable_output();

    disable_latch();
  }
}


#include "bitmap_1.h"
#include "bitmap_2.h"
#include "bitmap_3.h"

bool webusb_connected = false;
bool run_display = true;


void setup() {
  pinMode(PIN_BTN_1, INPUT_PULLUP);
  pinMode(PIN_BTN_2, INPUT_PULLUP);

  init_hub75e();
  
  usb_web.setLandingPage(&landingPage);
  usb_web.setLineStateCallback(line_state_callback);
  usb_web.setStringDescriptor("TinyUSB WebUSB LEDMatrix");
  usb_web.begin();

  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }

  memcpy(&vram[0], bitmap_2, 128 * 64 / 2);

  for (unsigned long t = millis(); millis() - t < 2000; ) {
    blink_led(100);
    TinyUSBDevice.task();
  }

}

void wait_nbytes_over_usbweb(uint8_t nbytes) {
  while (usb_web.available() < nbytes) {
    TinyUSBDevice.task();
    blink_led(50);
  }
}

void loop() {
  static unsigned long display_timer = 0;

  TinyUSBDevice.task();

  if (!TinyUSBDevice.mounted() || !webusb_connected) {
    static unsigned long changepage_timer = 0;
    blink_led(1000);

    if (millis() - changepage_timer > 7500 * 3) {
      memcpy((uint8_t*)&vram[0], bitmap_2, 4096);
      changepage_timer = millis();
    
    } else if (millis() - changepage_timer > 7500 * 2) {
      memcpy((uint8_t*)&vram[0], bitmap_3, 4096);

    } else if (millis() - changepage_timer > 7500 * 1) {
      for (int i = 0; (millis() - changepage_timer < 7500 * 2); i++) {
        show_bitmap(bitmap_1, 256, 128, i);
        TinyUSBDevice.task();
        blink_led(1000);
      }
    }

  } else {
    blink_led(250);
    process_usb();
  }

  if (run_display) {
    show_bitmap((uint8_t*)&vram[0], 128, 128, 0);
  }
}

void process_usb() {
  while (usb_web.available() > 0) {
    // TinyUSBDevice.task();
    uint8_t cmd = 0;
    usb_web.read(&cmd, 1);

    if (cmd == 0x01) {
      uint8_t offset_high = 0;
      uint8_t offset_low = 0;
      wait_nbytes_over_usbweb(2);
      usb_web.read(&offset_low, 1);
      usb_web.read(&offset_high, 1);
      uint16_t offset = offset_high << 8 | offset_low;
      uint8_t datalen = 0;
      wait_nbytes_over_usbweb(1);
      usb_web.read(&datalen, 1);
      wait_nbytes_over_usbweb(datalen);
      usb_web.read(&vram[0][offset], datalen);

    } else if (cmd == 0x02) {
      memset((uint8_t*)&vram, 0x00, sizeof(vram));

    } else if (cmd == 0x03) {
      run_display = false;
      show_blank_line(128);

    } else if (cmd == 0x04) {
      run_display = true;
    }
  }
}

void line_state_callback(bool connected) {
  if (connected) {
    usb_web.println("WebUSB interface connected !!");
    usb_web.flush();
    webusb_connected = true;
  } else {
    webusb_connected = false;
  }
}
