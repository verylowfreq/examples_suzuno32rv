/* SG-90サーボを駆動します。
PA0ピンに接続して、ハードウェアタイマー Timer2を使ってPWM信号を生成します。

ハードウェアタイマーは利用できるピンが決められています。ピンを変えたい場合はデータシートを参照してください。

システムクロックは144MHzにしてください。
*/

#include <HardwareTimer.h>

HardwareTimer Timer2(TIM2);

void setup() {
  servo_init();
}

void loop() {
  int i = 0;
  for (i = 0; i < 180; i++) {
    servo_set_angle(i);
    delay(10);
  }
  for (; i != 0; i--) {
    servo_set_angle(i);
    delay(10);
  }
}

void servo_init() {
  Timer2.pause();
  Timer2.setPrescaleFactor(1440);
  Timer2.setOverflow(2000);
  Timer2.setMode(TIM_CHANNEL_CH1, TIMER_OUTPUT_COMPARE_PWM1, PA0);
  Timer2.setCaptureCompare(TIM_CHANNEL_CH1, 1500 / 10);
  Timer2.refresh();
  Timer2.resume();
}

void servo_set_angle(int angle) {
  angle = constrain(angle, 0, 180);
  uint16_t compare_ticks = ((angle / 180.0) * (2400 - 500) + 500) / 10;

  Timer2.setCaptureCompare(TIM_CHANNEL_CH1, compare_ticks);
}
