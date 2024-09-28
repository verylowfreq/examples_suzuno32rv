# Suzuno32RV サンプルコード集 / Sample projects for Suzuno32RV

## led_blink : LEDの点滅

pinMode(), digitalWrite()


## led_flasher : LEDの複数点灯

pinMode(), digitalWrite(), LEDシールド


## i2c_oled : I2C接続のOLEDの表示

Wire, 外部ライブラリの利用


## servo : SG-90サーボの制御

HardwareTimer, PWM, SG-90


## servo : スライダー操作のロボットアーム

HardwareTimer, PWM, SG-90, analogRead()


## neopixel : NeoPixelの点灯（ライブラリを利用）

Adafruit_NeoPixelライブラリ


## neopixel by timer : NeoPixelの点灯（ハードウェアタイマーを利用）

SPL, Timer, GPIO


## tinyusb_usbd_keyboard : USBキーボードとして振舞う

TinyUSB, USB Device, Keyboard


## tinyusb_usbd_keyboard : MIDI入力デバイスとして振舞う

TinyUSB, USB Device, MIDI input


## tinyusb_usbd_webusb : WebUSBデバイスとして振舞う

TinyUSB, USB Device, WebUSB


## led_matrix_hub75e_webusb : LEDマトリクス HUB75e の駆動と、WebUSB制御

GPIO, TinyUSB, WebUSB


## tinyusb_thermalprinter : USB接続のサーマルプリンター ZJ-5890K を制御

TinyUSB, USB Host

## platformio_usb_imageviewer : USBメモリーに保存されたJPEG画像を液晶シールドへスライドショー表示（PlatformIOでビルド）

公式サンプルコード, PlatformIO, UNOシールド

