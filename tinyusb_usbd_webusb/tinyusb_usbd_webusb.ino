/* WebUSB対応デバイスとして振舞い、ブラウザからこのデバイスをコントロールできるようにします。
www/index.htmlをGoogle Chromeで開いてください。
*/


#include "Adafruit_TinyUSB.h"

#define PIN_LED PA5

Adafruit_USBD_WebUSB usb_web;
WEBUSB_URL_DEF(landingPage, 1 /*https*/, "example.tinyusb.org/webusb-serial/index.html");

bool is_web_usb_connected = false;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  
  usb_web.setStringDescriptor("Suzuno32RV WebUSB Sample");
  usb_web.setLineStateCallback(line_state_callback);
  usb_web.begin();
}

void loop() {
  TinyUSBDevice.task();

  if (TinyUSBDevice.mounted() && is_web_usb_connected) {
    if (usb_web.available() > 0) {
      uint8_t val = usb_web.read();
      update_led(val);
    }
  } else {
    update_led('0');
  }
}

void update_led(uint8_t val) {
  if (val == '1') {
    digitalWrite(PIN_LED, HIGH);
  } else if (val == '0') {
    digitalWrite(PIN_LED, LOW);
  }
}

void line_state_callback(bool connected) {
  is_web_usb_connected = connected;
}
