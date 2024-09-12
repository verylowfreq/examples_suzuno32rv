/*
Suzuno32RV上のLEDを点滅します。このLEDはPA5ピンに接続されていて、HIGHで点灯、LOWで消灯です。
 */


#define PIN_LED PA5

void setup() {
    pinMode(PIN_LED, OUTPUT);
}

void loop() {
    digitalWrite(PIN_LED, HIGH);
    delay(500);
    digitalWrite(PIN_LED, LOW);
    delay(500);
}
