/* USB接続したレシートプリンター ZJ-5890K で印刷をします。

このレシートプリンターは類似の商品が多数流通していますが、すべて同一の仕様なのかは不明です。

起動時（リセット時）に1回印刷、そのあと、PA0がGNDにつながるごとに印刷します。

USB Support で USBFS Host を選んでください。
*/

#include <Adafruit_TinyUSB.h>

constexpr int PIN_BTN_1 = PA0;

typedef struct
{
  tusb_desc_device_t desc_device;
  uint16_t manufacturer[32];
  uint16_t product[48];
  uint16_t serial[16];
  bool mounted;
} dev_info_t;

// CFG_TUH_DEVICE_MAX is defined by tusb_config header
dev_info_t dev_info[CFG_TUH_DEVICE_MAX] = {0};

Adafruit_USBH_Host USBHost;


//--------------------------------------------------------------------+
// setup() & loop()
//--------------------------------------------------------------------+
void setup()
{
  pinMode(PA5, OUTPUT);
  pinMode(PIN_BTN_1, INPUT_PULLUP);
  
  Serial1.begin(115200);
  Serial1.println("TinyUSB Host: Device Info Example");

  // Init USB Host on native controller roothub port0
  if (!USBHost.begin(0))
  {
    Serial1.printf("Failed to begin");
    while (true)
    {
      blink_led(50);
    }
  }
}

void loop()
{
  USBHost.task();
  Serial1.flush();
  blink_led(500);

  check_btn();
}

enum TP_STATE
{
  TP_NOT_READY,
  TP_READY,
  TP_SET_CONFIGURATION,
  TP_SET_CONFIGURATION_DONE,
  TP_SET_INTERFACE_DONE,
  TP_XFER_DONE
};

uint8_t cur_daddr = 0;
bool is_printing = false;
uint16_t total_sent_len = 0;

void check_btn() {
  static bool is_btn_pressing = false;

  bool pressing = digitalRead(PIN_BTN_1) == LOW;
  if (pressing && !is_btn_pressing) {
    // Start pressing
    is_btn_pressing = true;
    // Insert delay for deboucing
    delay(10);

  } else if (!pressing && is_btn_pressing) {
    // Pressing is ended
    is_btn_pressing = false;
    
    if (!is_printing && cur_daddr != 0) {
      is_printing = true;
      total_sent_len = 0;
      tuh_xfer_t data = {};
      data.daddr = cur_daddr;
      data.user_data = TP_SET_CONFIGURATION;
      thermalprinter_cb(&data);
    }
  }
}

void blink_led(unsigned int time_ms)
{
  static unsigned long timer = 0;
  static bool led_on = false;
  if (timer == 0)
  {
    pinMode(PA5, OUTPUT);
  }
  if (millis() - timer > time_ms)
  {
    if (led_on)
    {
      digitalWrite(PA5, LOW);
    }
    else
    {
      digitalWrite(PA5, HIGH);
    }
    led_on = !led_on;
    timer = millis();
  }
}


//--------------------------------------------------------------------+
// TinyUSB Host callbacks
//--------------------------------------------------------------------+
void print_device_descriptor(tuh_xfer_t *xfer);

void utf16_to_utf8(uint16_t *temp_buf, size_t buf_len);

void print_lsusb(void)
{
  bool no_device = true;
  for (uint8_t daddr = 1; daddr < CFG_TUH_DEVICE_MAX + 1; daddr++)
  {
    // TODO can use tuh_mounted(daddr), but tinyusb has an bug
    // use local connected flag instead
    dev_info_t *dev = &dev_info[daddr - 1];
    if (dev->mounted)
    {
      Serial1.printf("Device %u: ID %04x:%04x %s %s\r\n", daddr,
                     dev->desc_device.idVendor, dev->desc_device.idProduct,
                     (char *)dev->manufacturer, (char *)dev->product);

      no_device = false;
    }
  }

  if (no_device)
  {
    Serial1.println("No device connected (except hub)");
  }
}

// Invoked when device is mounted (configured)
void tuh_mount_cb(uint8_t daddr)
{
  Serial1.printf("Device attached, address = %d\r\n", daddr);

  dev_info_t *dev = &dev_info[daddr - 1];
  dev->mounted = true;

  // Get Device Descriptor
  // tuh_descriptor_get_device(daddr, &dev->desc_device, 18, print_device_descriptor, 0);

  cur_daddr = daddr;

  is_printing = true;
  total_sent_len = 0;
  tuh_xfer_t data = {};
  data.daddr = daddr;
  data.user_data = TP_SET_CONFIGURATION;
  thermalprinter_cb(&data);
}

/// Invoked when device is unmounted (bus reset/unplugged)
void tuh_umount_cb(uint8_t daddr)
{
  Serial1.printf("Device removed, address = %d\r\n", daddr);
  dev_info_t *dev = &dev_info[daddr - 1];
  dev->mounted = false;

  cur_daddr = 0;

  // print device summary
  print_lsusb();
}

#include <host/hcd.h>


tusb_desc_endpoint_t edpt_in;


void print_device_descriptor(tuh_xfer_t *xfer)
{
  if (XFER_RESULT_SUCCESS != xfer->result)
  {
    Serial1.printf("Failed to get device descriptor\r\n");
    return;
  }

  uint8_t const daddr = xfer->daddr;

  tuh_xfer_t data = {};
  data.daddr = daddr;
  data.user_data = TP_SET_CONFIGURATION;
  thermalprinter_cb(&data);
}

void data_sent_callback(tuh_xfer_t *xfer)
{
  Serial1.printf("data_sent_callback() called.\r\n");
  Serial1.printf("result = %d %s\r\n", xfer->result, xfer_result_to_str(xfer->result));
}

const char *xfer_result_to_str(xfer_result_t result)
{
  switch (result)
  {
  case XFER_RESULT_SUCCESS:
    return "SUCCESS";
  case XFER_RESULT_FAILED:
    return "FAILED";
  case XFER_RESULT_STALLED:
    return "STALLED";
  case XFER_RESULT_TIMEOUT:
    return "TIMEOUT";
  default:
    return "INVALID";
  }
}

void thermalprinter_cb(tuh_xfer_t* xfer)
{
  uint8_t daddr = xfer->daddr;
  int nextState = xfer->user_data;
  Serial1.printf("daddr = 0x%02x\r\n", daddr);
  switch (nextState)
  {
  case TP_SET_CONFIGURATION:
  {
    Serial1.printf("Try SET_CONFIGURATION...\r\n");
    if (!tuh_configuration_set(daddr, 1, thermalprinter_cb, TP_SET_CONFIGURATION_DONE))
    {
      Serial1.printf("tuh_configuration_set() failed\r\n");
    }
    else
    {
      Serial1.printf("tuh_configuration_set() success\r\n");
    }
    return;
  }
  case TP_SET_CONFIGURATION_DONE:
  {
    // Serial1.printf("SetConfiguration result = %d %s\r\n", xfer->result, xfer_result_to_str(xfer->result));

    // for (int i = 0; i < 100; i++) {
    //   USBHost.task();
    //   delay(1);
    // }

    // if (!tuh_interface_set(daddr, 0, 0, thermalprinter_cb, TP_SET_INTERFACE_DONE))
    // {
    //   Serial1.println("FAILED: calling tuh_interface_set() failed.");
    //   while (true)
    //   {
    //     blink_led(100);
    //   }
    // }
    // else
    // {
    //   Serial1.println("SUCCESS: calling tuh_interface_set() success.");
    // }
    // return;
  }
  case TP_SET_INTERFACE_DONE:
  {
    // Serial1.printf("SetInterface result = %d %s\r\n", xfer->result, xfer_result_to_str(xfer->result));
    if (xfer->result == XFER_RESULT_FAILED)
    {
      while (true)
      {
        blink_led(100);
      }
    }
    tusb_desc_endpoint_t ep_desc = {};
    ep_desc.bEndpointAddress = 0x01;
    ep_desc.wMaxPacketSize = 64;
    ep_desc.bmAttributes.xfer = TUSB_XFER_BULK;
    // memcpy(&ep_desc, &edpt_in, sizeof(tusb_desc_endpoint_t));
    Serial1.printf("ep_addr=0x%02x\r\n", ep_desc.bEndpointAddress);
    Serial1.printf("wMaxPacketSize=%d\r\n", ep_desc.wMaxPacketSize);
    Serial1.printf("Call tuh_edpt_open()\r\n");
    if (!tuh_edpt_open(daddr, &ep_desc))
    {
      Serial1.println("FAILED: calling tuh_edpt_open() failed.");
      while (true)
      {
        blink_led(100);
      }
    }
    else
    {
      Serial1.println("SUCCESS: calling tuh_edpt_open() success.");
    }

    static const char *s = "\n"
                    "####        --------        ####\n"
                    "  Thermal printer control       \n"
                    "    from RISC-V MCU !           \n"
                    "         -- Suzuno32RV --       \n"
                    "####        --------        ####\n\n\n";
    uint16_t datalen = strlen(s) + 1;

    if (total_sent_len < datalen)
    {
      uint16_t sendlen = 64;
      if (sendlen > datalen) {
        sendlen = datalen;
      }
      uint8_t data[64];
      memcpy(data, s + total_sent_len, sendlen);

      datalen -= sendlen;
      total_sent_len += sendlen;
      
      tuh_xfer_t edpt_xfer = {};
      edpt_xfer.daddr = daddr;
      edpt_xfer.ep_addr = 0x01;
      edpt_xfer.buflen = sendlen;
      edpt_xfer.buffer = data;
      edpt_xfer.complete_cb = thermalprinter_cb;
      edpt_xfer.user_data = TP_SET_INTERFACE_DONE;

      Serial1.println("Send data");

      if (!tuh_edpt_xfer(&edpt_xfer))
      {
        Serial1.printf("tuh_edpt_xfer() failed\r\n");
      }
      else
      {
        Serial1.printf("xfer has began\r\n");
      }
    } else {
      is_printing = false;
    }
  }
  }
}
