#include <Arduino.h>


#include "debug.h"
#include "stdio.h"
#include "string.h"
#include "picojpeg.h"
#include "Udisk_Operation.h"

#include <SPI.h>
#include "LCD.h"
// #include "image.h"
// RGB565 binary
// const uint8_t image[] __attribute__(section(.data_flash1)) = {
//   ...
// };

const int WIDTH = 240;
const int HEIGHT = 320;


bool is_file_exits(const char* filename);
void show_image_from_usbms(const char* filename);
void delay_sec(unsigned int time_sec);

void setup() {

    Delay_Init( );

    Serial.begin(115200);
    Serial.printf("SystemClk:%u\r\n", SystemCoreClock);

    // Initi SPI
    SPI.begin();
    SPI.beginTransaction(
        SPISettings(8000000, MSBFIRST, SPI_MODE0)
    );

    // Init LCD
    Tft.lcd_init();
    Tft.setRotation(Rotation_0_D);

    Tft.lcd_clear_screen(0x80);
    Tft.lcd_clear_screen(0xff);
    Tft.lcd_clear_screen(0x00);


    Serial.printf("Start USB demo...\n");

    /* General USB Host UDisk Operation Initialization */
    Udisk_USBH_Initialization( );

    while (true) {
      for (int i = 1; i < 10; i++) {
        char buf[32] = { 0 };
        snprintf(buf, sizeof(buf) - 1, "/IMAGE%d.JPG", i);
        if (is_file_exits(buf)) {
          show_image_from_usbms(buf);
          delay_sec(2);
        }
      }
    }
}

void loop() {
  // NOP
  delay(100);
}


bool is_file_exits(const char* filename) {
  
  uint8_t ret;

  
  ret = UDisk_USBH_DiskReady();

  if (ret != DISK_READY/*|| UDisk_Opeation_Flag != 1*/) {
    Serial.printf("Disk not ready.\n");
    return false;
  }

  UDisk_Opeation_Flag = 0;

  strcpy((char*)mCmdParam.Open.mPathName, filename);

  ret = CHRV3FileOpen();

  if (ret == ERR_MISS_FILE) {
    Serial.printf("File \"%s\" not found.\n", filename);
    return false;
  }

  Serial.printf("File \"%s\" opened.\n", filename);

  ret = CHRV3FileClose();
  mStopIfError(ret);

  return true;
}


uint32_t image_filesize;
uint32_t image_readlen;

unsigned char picojpeg_callback(unsigned char* pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data) {
  uint8_t ret;
  uint32_t copylen = 200;
  uint32_t file_left_size = image_filesize - image_readlen;
  if (copylen > buf_size) {
    copylen = buf_size;
  }
  if (copylen > file_left_size) {
    copylen = file_left_size;
  }

  mCmdParam.ByteRead.mByteCount = copylen;
  if (mCmdParam.ByteRead.mByteCount > file_left_size) {
    mCmdParam.ByteRead.mByteCount = file_left_size;
  }
  mCmdParam.ByteRead.mByteBuffer = Com_Buffer;

  ret = CHRV3ByteRead();

  uint16_t actual_read_len = mCmdParam.ByteRead.mByteCount;
  if (actual_read_len == 0) {
    Serial.printf("File read %d bytes\n", actual_read_len);
    *pBytes_actually_read = 0;
    return -1;    
  }
  
  image_readlen += actual_read_len;

  memcpy(pBuf, mCmdParam.ByteRead.mByteBuffer, copylen);
  *pBytes_actually_read = copylen;

  return 0;
}


void show_image_from_usbms(const char* filename) {
  uint8_t ret;

  
  ret = UDisk_USBH_DiskReady();

  if (ret != DISK_READY/*|| UDisk_Opeation_Flag != 1*/) {
    Serial.printf("Disk not ready.\n");
    return;
  }

  UDisk_Opeation_Flag = 0;

  strcpy((char*)mCmdParam.Open.mPathName, filename);

  ret = CHRV3FileOpen();

  if (ret == ERR_MISS_FILE) {
    Serial.printf("File \"%s\" not found.\n", filename);
    return;
  }

  Serial.printf("File \"%s\" opened.\n", filename);

  ret = CHRV3FileQuery();
  mStopIfError(ret);

  image_filesize = CHRV3vFileSize;
  Serial.printf("File size: %u bytes\n", image_filesize);
  image_readlen = 0;

  pjpeg_image_info_t pjpeg_info;
  ret = pjpeg_decode_init(&pjpeg_info, picojpeg_callback, NULL, 0);
  if (ret == PJPG_UNSUPPORTED_MODE) {
    Serial.printf("JPEG progressive mode is not supported.\n");
    ret = CHRV3FileClose();
    mStopIfError(ret);
    return;
  }

  int count = 0;
  uint32_t imageWidth = pjpeg_info.m_width;
  uint32_t imageHeight = pjpeg_info.m_height;
  uint8_t mcuWidth = pjpeg_info.m_MCUWidth;
  uint8_t mcuHeight = pjpeg_info.m_MCUHeight;
  uint32_t cur_x = 0;
  uint32_t cur_y = 0;
  
  while (true) {
      ret = pjpeg_decode_mcu();
      count += 1;
      if (ret == PJPG_NO_MORE_BLOCKS) {
          Serial.printf("All blocks decoded.\n");
          break;
      }

      uint8_t* pixelR = pjpeg_info.m_pMCUBufR;
      uint8_t* pixelG = pjpeg_info.m_pMCUBufG;
      uint8_t* pixelB = pjpeg_info.m_pMCUBufB;

      Tft.LCD_SetWindow(cur_x, cur_y, cur_x + mcuWidth, cur_y + mcuHeight);
			Tft.lcd_write_byte(0x2C, LCD_CMD);
			__LCD_DC_SET();
			__LCD_CS_CLR();

      for (int y = 0; y < mcuHeight; y++) {
          for (int x = 0; x < mcuWidth; x++) {
              int offset = y * mcuWidth + x;
              uint8_t r = pixelR[offset];
              uint8_t g = pixelG[offset];
              uint8_t b = pixelB[offset];
              r = r >> 3;
              g = g >> 2;
              b = b >> 3;

              uint16_t color = (r << 11) | (g << 5) | b;

              __LCD_WRITE_BYTE(color >> 8);
              __LCD_WRITE_BYTE(color & 0xFF);
          }
      }

      cur_x += mcuWidth;
      if (cur_x >= imageWidth) {
          cur_y += mcuHeight;
          cur_x = 0;
      }
      
			__LCD_CS_SET();
  }

  Serial.printf("Total read len %u bytes\n", image_readlen);

  ret = CHRV3FileClose();
  mStopIfError(ret);
}


void delay_sec(unsigned int time_sec) {
  for (unsigned int i = 0; i < time_sec * 5; i++) {
    Delay_Ms(1000);
  }
}
