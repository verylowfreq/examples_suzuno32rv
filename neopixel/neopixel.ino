/* NeoPixel フルカラーLEDを制御します。
ここではNeoPixelに送信するコマンドを生成する方法として、タイマーでタイミングをとるようにしています。
PA1ピンにNeoPixel信号線を接続してください。
システムクロックは144MHzにしてください。

このコードでは Arduino環境のdigitalwrite()やHardwareTimerではなく、基底のSDKの関数を直接呼び出しています。
*/


void setup() {
  np_setup();

  np_clear();
  np_show();

  for (int i = 0; i < 4; i++) {
    np_clear();
    np_show();
    delay(100);
    np_set(0, 128,128,128);
    np_show();
    delay(100);
  }
}

void loop() {
  showSinglePixel();
  showAllPixels();
  showHue();
}


constexpr int NEOPIXEL_COUNT = 37;

constexpr int NEOPIXEL_BUFFER_LENGTH = NEOPIXEL_COUNT * 3;
uint8_t neopixel_buffer[NEOPIXEL_BUFFER_LENGTH];

void np_clear() {
  for (int i = 0; i < NEOPIXEL_BUFFER_LENGTH; i++) {
    neopixel_buffer[i] = 0;
  }
}

void np_set(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
  neopixel_buffer[index * 3 + 0] = g;
  neopixel_buffer[index * 3 + 1] = r;
  neopixel_buffer[index * 3 + 2] = b;
}

inline void np_pin_high() {
  GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
}

inline void np_pin_low() {
  GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET);
}

void np_pin_init() {
  // GPIO Aにクロックを供給して、機能を有効にする
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  // GPIO A 0を出力として初期化
  GPIO_InitTypeDef GPIO_InitStructure = { };
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void np_timer_init() {
  // タイマー2を有効にする
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  // 分周1なしでカウントアップさせる
  TIM_TimeBaseInitTypeDef tim_init = { };
  tim_init.TIM_Period = 0xffff;
  tim_init.TIM_Prescaler = 0;
  tim_init.TIM_CounterMode= TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &tim_init);
  TIM_Cmd(TIM2, ENABLE);
}

void np_send_reset() {
  np_pin_low();
  delayMicroseconds(50);
}


#define USEC_PER_CYCLE (1000000.0 / 144000000.0)
#define NP_T0H_LOOPCNT (int)(0.15 / USEC_PER_CYCLE)
#define NP_T0L_LOOPCNT (int)(0.65 / USEC_PER_CYCLE)
#define NP_T1H_LOOPCNT (int)(0.65 / USEC_PER_CYCLE)
#define NP_T1L_LOOPCNT (int)(0.15 / USEC_PER_CYCLE)

inline void np_send_zero() {
  TIM_SetCounter(TIM2, 0);
  np_pin_high();
  while (TIM_GetCounter(TIM2) < NP_T0H_LOOPCNT) { }
  TIM_SetCounter(TIM2, 0);
  np_pin_low();
  while (TIM_GetCounter(TIM2) < NP_T0L_LOOPCNT) { }
}

inline void np_send_one() {
  TIM_SetCounter(TIM2, 0);
  np_pin_high();
  while (TIM_GetCounter(TIM2) < NP_T1H_LOOPCNT) { }
  TIM_SetCounter(TIM2, 0);
  np_pin_low();
  while (TIM_GetCounter(TIM2) < NP_T1L_LOOPCNT) { }
}

void np_show() {
  __disable_irq();
  np_send_reset();
  for (int i = 0; i < NEOPIXEL_BUFFER_LENGTH; i++) {
    uint8_t b = neopixel_buffer[i];
    for (int i = 0; i < 8; i++) {
      if (b & 0x80) {
        np_send_one();
      } else {
        np_send_zero();
      }
      b = b << 1;
    }
  }
  __enable_irq();
}

void np_setup() {
  np_pin_init();
  np_timer_init();
}

byte hueCurve(int x) {
  x += 1024/3;
  x = x % 1024;
  return max(255-abs(255-x*(255.0*3/1024)), 0.0);
}

uint32_t getHue(int hue) {
  byte r = hueCurve(hue);
  byte g = hueCurve(hue+(1024*2)/3);
  byte b = hueCurve(hue+1024/3);
  return r << 16 | g << 8 | b;
}


void setAllPixels(uint32_t color) {
  uint8_t r = color >> 16;
  uint8_t g = color >> 8;
  uint8_t b = color;
  for (int i = 0; i < 37; i++) {
    np_set(i, r, g, b);
  }
  np_show();
}

void showAllPixels() {
  setAllPixels(0x101010);
  delay(500);
  setAllPixels(0x100000);
  delay(500);
  setAllPixels(0x001000);
  delay(500);
  setAllPixels(0x000010);
  delay(500);
}

void showSinglePixel() {
  for (unsigned long t = millis(); millis() - t < 1000;) {
    for (int i = 0; i < 37; i++) {
      np_clear();
      uint8_t r = i % 3 == 0 ? 128 : 0;
      uint8_t g = i % 3 == 1 ? 128 : 0;
      uint8_t b = i % 3 == 2 ? 128 : 0;
      np_set(i, r, g, b);
      np_show();
      delay(80);
    }
  }
}

void showHue() {
  int cnt = 0;
  for (unsigned long t = millis(); millis() - t < 4000;) {
    for (int i = 0; i < NEOPIXEL_COUNT; i++) {
      uint32_t color = getHue(cnt);
      uint8_t r = color >> 16;
      uint8_t g = color >> 8;
      uint8_t b = color & 0xff;
      np_set(i, r / 4, g / 4, b / 4);
      cnt += 1;
    }
    np_show();
    delay(50);
  }
}
