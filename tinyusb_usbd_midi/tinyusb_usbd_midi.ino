/* MIDI入力デバイスとして振舞います。
PA15, PB3, PB4, PB5ピンにスイッチを、押下時にGNDへつながるように接続してください。
スイッチを押すと、それぞれノートの入力が送信されます。

Arduino IDEで、ツール → USB Support → Adafruit TinyUSB with USBD を選択してください。

ライブラリのインストールが必要です。"MIDI Library by Francois Best, lathoub" を導入してください。
*/


#define PIN_KEY_1 PA15
#define PIN_KEY_2 PB3
#define PIN_KEY_3 PB4
#define PIN_KEY_4 PB5


#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

// USB MIDI object
Adafruit_USBD_MIDI usb_midi;

MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);

void setup() {
    
  pinMode(PIN_KEY_1, INPUT_PULLUP);
  pinMode(PIN_KEY_2, INPUT_PULLUP);
  pinMode(PIN_KEY_3, INPUT_PULLUP);
  pinMode(PIN_KEY_4, INPUT_PULLUP);

  usb_midi.setStringDescriptor("TinyUSB MIDI");
  MIDI.begin(MIDI_CHANNEL_OMNI);
}

bool prev_key_1 = false;
bool prev_key_2 = false;
bool prev_key_3 = false;
bool prev_key_4 = false;

void loop() {
  TinyUSBDevice.task();
  if (TinyUSBDevice.mounted()) {
    if (digitalRead(PIN_KEY_1) == LOW) {
        if (!prev_key_1) {
            MIDI.sendNoteOn(60, 127, 1);
        }
        prev_key_1 = true;
    } else {
        if (prev_key_1) {
            MIDI.sendNoteOff(60, 0, 1);
        }
        prev_key_1 = false;
    }
    
    if (digitalRead(PIN_KEY_2) == LOW) {
        if (!prev_key_2) {
            MIDI.sendNoteOn(62, 127, 1);
        }
        prev_key_2 = true;
    } else {
        if (prev_key_2) {
            MIDI.sendNoteOff(62, 0, 1);
        }
        prev_key_2 = false;
    }
    
    if (digitalRead(PIN_KEY_3) == LOW) {
        if (!prev_key_3) {
            MIDI.sendNoteOn(64, 127, 1);
        }
        prev_key_3 = true;
    } else {
        if (prev_key_3) {
            MIDI.sendNoteOff(64, 0, 1);
        }
        prev_key_3 = false;
    }
    
    if (digitalRead(PIN_KEY_4) == LOW) {
        if (!prev_key_4) {
            MIDI.sendNoteOn(65, 127, 1);
        }
        prev_key_4 = true;
    } else {
        if (prev_key_4) {
            MIDI.sendNoteOff(65, 0, 1);
        }
        prev_key_4 = false;
    }
  }
}
