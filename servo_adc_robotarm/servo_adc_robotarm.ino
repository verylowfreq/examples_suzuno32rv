/*
  可変抵抗入力端子： PB0, PB1, PA6, PA7
  サーボモーター信号端子（出力）: PA0, PA1, PA2, PA3

  動作周波数は144MHz決め打ちです。
  サーボモーターを4つ駆動するので、電源は出力に余裕のあるACアダプターがよいです。
 */

#include <HardwareTimer.h>

HardwareTimer Timer2(TIM2);

void servo_init() {
  Timer2.pause();
  Timer2.setPrescaleFactor(1440);
  Timer2.setOverflow(2000);
  Timer2.setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA0);
  Timer2.setMode(2, TIMER_OUTPUT_COMPARE_PWM1, PA1);
  Timer2.setMode(3, TIMER_OUTPUT_COMPARE_PWM1, PA2);
  Timer2.setMode(4, TIMER_OUTPUT_COMPARE_PWM1, PA3);
  Timer2.setCaptureCompare(1, 1500 / 10);
  Timer2.setCaptureCompare(2, 1500 / 10);
  Timer2.setCaptureCompare(3, 1500 / 10);
  Timer2.setCaptureCompare(4, 1500 / 10);
  Timer2.refresh();
  Timer2.resume();
}

void servo_set(uint8_t ch, uint8_t angle) {
  angle = constrain(angle, 0, 180);
  uint16_t high_time = ((angle / 180.0) * (2400 - 500) + 500) / 10;
  switch (ch) {
  case 1:
    Timer2.setCaptureCompare(1, high_time);
    break;
  case 2:
    Timer2.setCaptureCompare(2, high_time);
    break;
  case 3:
    Timer2.setCaptureCompare(3, high_time);
    break;
  case 4:
    Timer2.setCaptureCompare(4, high_time);
    break;
  default:
    break;
  }
}

void setup() {
  Serial.println("Initializing..");

  servo_init();

  Serial.println("Ready.");
}

void loop() {
  static uint16_t prev_adcvalue_list[4] = { 4096 / 2, 4096 / 2, 4096 / 2, 4096 / 2 };

  uint16_t adcvalue_list[4];
  adcvalue_list[0] = analogRead(PB0);
  adcvalue_list[1] = analogRead(PB1);
  adcvalue_list[2] = analogRead(PA6);
  adcvalue_list[3] = analogRead(PA7);
  uint16_t filtered_adcvalue_list[4];
  for (uint8_t i = 0; i < 4; i++) {
    filtered_adcvalue_list[i] = prev_adcvalue_list[i] * 0.1f + adcvalue_list[i] * 0.9f;
  }

  uint8_t angle_list[4];
  for (uint8_t i = 0; i < 4; i++) {
    angle_list[i] = 180 * filtered_adcvalue_list[i] / 4096;
  }

  for (uint8_t i = 0; i < 4; i++) {
    prev_adcvalue_list[i] = filtered_adcvalue_list[i];
  }

  for (uint8_t i = 0; i < 4; i++) {
    servo_set(i + 1, angle_list[i]);
  }

  delay(3);
}
