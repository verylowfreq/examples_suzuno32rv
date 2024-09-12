/*
Suzuno32RVのArduino UNOヘッダーに引き出されたピンすべてを順番にオン・オフします。
LEDを接続したりLEDシールドを使うと、順番に点灯できます。
HIGHで点灯、LOWで消灯です。
*/

int PINS_LED[] = {
    PA10, PA9, PA8, PB12, PA15, PB3, PB4, PB5, PB11, PB10, PA4, PA7, PA6, PA5, PB9, PB8,
    PA0, PA1, PA2, PA3, PB0, PB1
};
size_t PINS_LED_COUNT = sizeof(PINS_LED) / sizeof(PINS_LED[0]);

void setup() {
    for (size_t i = 0; i < PINS_LED_COUNT; i++) {
        pinMode(PINS_LED[i], OUTPUT);
    }
}

void loop() {
    for (size_t i = 0; i < PINS_LED_COUNT; i++) {
        digitalWrite(PINS_LED[i], HIGH);
        delay(200);
        digitalWrite(PINS_LED[i], LOW);
    }
}
