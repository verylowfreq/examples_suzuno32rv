/* USBキーボードとして振舞います。
PA15, PB3, PB4, PB5ピンにスイッチを、押下時にGNDへつながるように接続してください。
スイッチを押すと、それぞれ 'a', 'b', 'c', 'd' が入力されます。

Arduino IDEで、ツール → USB Support → Adafruit TinyUSB with USBD を選択してください。
*/

#include <Adafruit_TinyUSB.h>

#define PIN_KEY_1 PA15
#define PIN_KEY_2 PB3
#define PIN_KEY_3 PB4
#define PIN_KEY_4 PB5


uint8_t const desc_keyboard_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()
};

Adafruit_USBD_HID usb_keyboard;

void setup() {
    
  // HID Keyboard
  usb_keyboard.setPollInterval(2);
  usb_keyboard.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
  usb_keyboard.setReportDescriptor(desc_keyboard_report, sizeof(desc_keyboard_report));
  usb_keyboard.setStringDescriptor("TinyUSB HID Keyboard");
  usb_keyboard.begin();

  pinMode(PIN_KEY_1, INPUT_PULLUP);
  pinMode(PIN_KEY_2, INPUT_PULLUP);
  pinMode(PIN_KEY_3, INPUT_PULLUP);
  pinMode(PIN_KEY_4, INPUT_PULLUP);
}

void loop() {
  TinyUSBDevice.task();

  if (TinyUSBDevice.mounted() && usb_keyboard.ready()) {
    // USB接続されていて、かつ、新しいキー入力の送信準備ができているとき

    uint8_t modifiers = 0x00;
    uint8_t keycodes[6] = { };
    if (digitalRead(PIN_KEY_1) == LOW) {
        keycodes[0] = 0x04;
    }
    if (digitalRead(PIN_KEY_2) == LOW) {
        keycodes[0] = 0x05;
    }
    if (digitalRead(PIN_KEY_3) == LOW) {
        keycodes[0] = 0x06;
    }
    if (digitalRead(PIN_KEY_4) == LOW) {
        keycodes[0] = 0x07;
    }
    usb_keyboard.keyboardReport(0, modifiers, keycodes);
  }
}
