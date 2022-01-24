#include "Config.h"
#include "ConfigReg.h"

#if ENABLE_DISPLAY
#include "Display.h"

#include "SPI.h"
#include <TJpg_Decoder.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <lvgl.h>


ConfigUInt8 configDisplayBrightness(FST("LCD Brightness"), 200, FST("Brightness of LCD backlight"), nullptr, nullptr, nullptr, setDisplayBrightness);
ConfigUInt16Array configTouchCalibration(FST("Touch Calibration"), 8,0,0,0,0,0,0,0,0, CVF_HIDDEN);

extern ConfigStr configSSID;

static bool isDisplayConfigured = false;
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9488  _panel_instance;
  lgfx::Bus_SPI        _bus_instance;
  lgfx::Light_PWM      _light_instance; 
  lgfx::Touch_XPT2046  _touch_instance;

public:
  LGFX(void) {
    
    { // Set the bus control.
      auto cfg = _bus_instance.config();    // Gets the structure for bus configuration.

      cfg.spi_host = HSPI_HOST;     // Select the SPI to use (VSPI_HOST or HSPI_HOST)
      cfg.spi_mode = 0;             // Set SPI communication mode (0 ~ 3)
      cfg.freq_write = 40000000;    // SPI clock at the time of transmission (up to 80MHz, rounded to the value obtained by dividing 80MHz by an integer)
      cfg.freq_read  = 16000000;    // SPI clock when receiving
      cfg.spi_3wire  = false;        // Set true when receiving with MOSI pin
      cfg.use_lock   = true;        // Set to true when using transaction lock
      cfg.dma_channel = 1;          // Set the DMA channel (1 or 2. 0=disable)   使用するDMAチャンネルを設定 (0=DMA不使用)
      cfg.pin_sclk = LCD_SCK_PIN;   // Set SPI SCLK pin number
      cfg.pin_mosi = LCD_MOSI_PIN;  // Set SPI MOSI pin number
      cfg.pin_miso = LCD_MISO_PIN;  // Set SPI MISO pin number (-1 = disable)
      cfg.pin_dc   = LCD_DC_PIN;    // Set SPI D / C pin number (-1 = disable)    
      _bus_instance.config(cfg);    // The set value is reflected on the bus.
      _panel_instance.setBus(&_bus_instance); // Set the bus on the panel.
    }
    { // Set the display panel control.
    auto cfg = _panel_instance.config();    // Gets the structure for display panel settings.

      cfg.pin_cs           =    LCD_CS_PIN;  // Pin number to which CS is connected (-1 = disable)
      cfg.pin_rst          =    -1;  // Pin number to which RST is connected (-1 = disable)
      cfg.pin_busy         =    -1;  // Pin number to which BUSY is connected (-1 = disable)

      // * The following setting values ​​are set to general initial values ​​for each panel, so comment out any unknown items and try them out.

      cfg.memory_width     =   320;  // Maximum width supported by driver IC
      cfg.memory_height    =   480;  // Maximum height supported by driver IC
      cfg.panel_width      =   320;  // Actually displayable width
      cfg.panel_height     =   480;  // Actually displayable height
      cfg.offset_x         =     0;  // Amount of X-direction offset of the panel
      cfg.offset_y         =     0;  // Amount of Y-direction offset of the panel
      cfg.offset_rotation  =     0;  // Offset of values ​​in the direction of rotation 0 ~ 7 (4 ~ 7 are upside down)
      cfg.dummy_read_pixel =     8;  // Number of dummy read bits before pixel reading
      cfg.dummy_read_bits  =     1;  // Number of bits of dummy read before reading data other than pixels
      cfg.readable         =  true;  // Set to true if data can be read
      cfg.invert           = false;  // Set to true if the light and darkness of the panel is reversed
      cfg.rgb_order        = false;  // Set to true if the red and blue of the panel are swapped
      cfg.dlen_16bit       = false;  // Set to true for panels that send data length in 16-bit units
      cfg.bus_shared       =  true;  // Set to true if the bus is shared with the SD card (bus control is performed with drawJpgFile etc.)

      _panel_instance.config(cfg);
    }    
//*
    { // Set the backlight control. (Delete if not needed)
      auto cfg = _light_instance.config();    // Gets the structure for the backlight setting.

      cfg.pin_bl = LCD_LED_PIN;     // Pin number to which the backlight is connected
      cfg.invert = false;           // True if you want to invert the brightness of the backlight
      cfg.freq   = 44100;           // Backlight PWM frequency
      cfg.pwm_channel = 7;          // PWM channel number to use

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);  // Set the backlight on the panel.
    }
//*/

//*
    { // Set the touch screen control. (Delete if not needed)
      auto cfg = _touch_instance.config();

      cfg.x_min      = 0;    // Minimum X value (raw value) obtained from touch screen
      cfg.x_max      = LCD_WIDTH-1;  // Maximum X value (raw value) obtained from touch screen
      cfg.y_min      = 0;    // Minimum Y value (raw value) obtained from touch screen
      cfg.y_max      = LCD_HEIGHT-1;  // Maximum Y value (raw value) obtained from touch screen
      cfg.pin_int    = TOUCH_CS_IRQ;   // Pin number to which INT is connected (-1 = disable)
      cfg.bus_shared = true; // Set to true if you are using the same bus as the screen
      cfg.offset_rotation = 0;// Adjustment when the display and touch orientation do not match Set with a value from 0 to 7

// For SPI connection
      cfg.spi_host = HSPI_HOST;// Select the SPI to use (HSPI_HOST or VSPI_HOST)
      cfg.freq = 1000000;     // Set SPI clock
      cfg.pin_sclk = LCD_SCK_PIN;    // Pin number to which SCLK is connected
      cfg.pin_mosi = LCD_MOSI_PIN;   // Pin number to which MOSI is connected
      cfg.pin_miso = LCD_MISO_PIN;   // Pin number to which MISO is connected
      cfg.pin_cs   =  TOUCH_CS_PIN;  // Pin number to which CS is connected

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);  //Set the touch screen on the panel.
    }
//*/

    setPanel(&_panel_instance); // Set the panel to be used.
  }
};

LGFX tft = LGFX();

uint16_t* tft_buffer;
bool      buffer_loaded = false;

bool has_touch = false;
uint32_t next_display_update_ms = 0;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ LCD_WIDTH * 10 ];

bool setDisplayBrightness(uint8_t val, void* cbData/*=nullptr*/) {
  if (isDisplayConfigured) { tft.setBrightness(val); }
  return false;
}

/* Display flushing */
void lv_display_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
   uint32_t w = ( area->x2 - area->x1 + 1 );
   uint32_t h = ( area->y2 - area->y1 + 1 );

   tft.startWrite();
   tft.setAddrWindow( area->x1, area->y1, w, h );
   // tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
   tft.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
   tft.endWrite();

   lv_disp_flush_ready( disp );
}

/*Read the touchpad*/
void lv_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
   uint16_t touchX=0, touchY=0;
   bool touched = tft.getTouch( &touchX, &touchY);
   if( !touched ) {
      data->state = LV_INDEV_STATE_REL;
   } else {
      data->state = LV_INDEV_STATE_PR;

      /*Set the coordinates*/
      data->point.x = touchX;
      data->point.y = touchY;
      //DEBUG_printf(FST("Touch X:%d y:%d\n"), touchX, touchY);
   }
}


// =======================================================================================
// This function will be called during decoding of the jpeg file
// =======================================================================================
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  tft.pushImage(x, y, w, h, bitmap);

  // Return 1 to decode next block
  return 1;
}

void calibrateTouch() {
  bool touch_configured = false;
  uint16_t* ctc = configTouchCalibration.get();
  for (size_t n=0; n<configTouchCalibration.size(); n++) {
    if (ctc[n] != 0) {
      touch_configured = true;
      break;
    }
  }
  if (touch_configured) {
      tft.setTouchCalibrate(configTouchCalibration.get());
      has_touch = true;
  } else {
    // data not valid. recalibrate
    DEBUG_println(F("Calibrating touch"));
    tft.setTextSize((std::max(tft.width(), tft.height()) + 255) >> 8);
    if (tft.width() < tft.height()) tft.setRotation(tft.getRotation() ^ 1);
    tft.setTextDatum(TC_DATUM);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString(F("Touch Calibration"), LCD_WIDTH/2, 100);
    tft.drawString(F("Touch red corners with white arrow."), LCD_WIDTH/2, 140);
    uint16_t tc[8] = {0};
    tft.calibrateTouch(tc, TFT_WHITE, TFT_RED, std::max(tft.width(), tft.height()) >> 3);
    for (size_t n=0; n<configTouchCalibration.size(); n++) {
      // DEBUG_printf(FST("%d "), config.touchCalibartionData[n]);      
      if (tc[n] > 1) {
        touch_configured = true;
        DEBUG_println(F("Calibration valid"));
        configTouchCalibration.set(tc);
        saveConfig();
        has_touch = true;
        break;
      }
    }
  }
  if (!touch_configured) {
    // This will only work if touch was never calibrated.
    DEBUG_println(F("Touch controller not found"));
    //setError(TOUCH_CONTROLLER_NOT_FOUND);
  }
}

/*
size_t getTouchCalibrationJson(char* buffer, size_t bSize) {
  return snprintf_P(buffer, bSize, FST("[%d, %d, %d, %d, %d, %d]"), 
    config.touchCalibartionData[0], config.touchCalibartionData[1],
    config.touchCalibartionData[2], config.touchCalibartionData[3],
    config.touchCalibartionData[4], config.touchCalibartionData[5]);
}
*/

void displaySetup() {
  if (LCD_LED_PIN > -1) {
    pinMode(LCD_LED_PIN, OUTPUT);
    digitalWrite(LCD_LED_PIN, HIGH);
    ledcSetup(LCD_LED_PWM_CHANNEL, 2000, 10);
    ledcAttachPin(LCD_LED_PIN, LCD_LED_PWM_CHANNEL);
    // At first set brightness to 100% to show display is working
    ledcWrite(LCD_LED_PWM_CHANNEL, 1023); 
  }
  tft.init();
  tft.setRotation(3);
  tft.setBrightness(configDisplayBrightness.get());
  calibrateTouch();

  lv_init();
  lv_disp_draw_buf_init( &draw_buf, buf, NULL, LCD_WIDTH * 10 );

   /*Initialize the display*/
   static lv_disp_drv_t disp_drv;
   lv_disp_drv_init( &disp_drv );
   /*Change the following line to your display resolution*/
   disp_drv.hor_res = LCD_WIDTH;
   disp_drv.ver_res = LCD_HEIGHT;
   disp_drv.flush_cb = lv_display_flush;
   disp_drv.draw_buf = &draw_buf;
   lv_disp_drv_register( &disp_drv );

   /*Initialize the (dummy) input device driver*/
   static lv_indev_drv_t indev_drv;
   lv_indev_drv_init( &indev_drv );
   indev_drv.type = LV_INDEV_TYPE_POINTER;
   indev_drv.read_cb = lv_touchpad_read;
   lv_indev_drv_register( &indev_drv );
   isDisplayConfigured = true;
}

static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;

        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
    }
}

void displayBootScreen() {
  tft.fillScreen(TFT_BLACK);

    // The byte order can be swapped (set true for TFT_eSPI)
  TJpgDec.setSwapBytes(true);

  // The jpeg decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);

  // Draw the logo
//  TJpgDec.drawJpg((TFT_WIDTH-240)/2, 40, apollo_logo, sizeof(apollo_logo));

  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(F("ESP32 ROS Remote"), LCD_WIDTH/2, 2);

  char buffer[64];
  tft.setCursor(0, 20);
  tft.setTextSize(1);
  tft.setFont(&fonts::Font4);
  sprintf(buffer, FST("Connecting to: %s"), configSSID.get());
  tft.drawString(buffer, LCD_WIDTH/2, 140);


/*
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(F("12345678"), 0, 20);
  tft.drawString(F("1234567"), 0, 40);
  tft.drawString(F("123456"), 0, 60);
  tft.drawString(F("12345"), 0, 80);
  tft.drawString(F("1234"), 0, 100);

  tft.setTextDatum(TR_DATUM);
  tft.drawString(F("12345678"), LCD_WIDTH-1, 120);
  tft.drawString(F("1234567"), LCD_WIDTH-1, 140);
  tft.drawString(F("123456"), LCD_WIDTH-1, 160);
  tft.drawString(F("12345"), LCD_WIDTH-1, 180);
  tft.drawString(F("1234"), LCD_WIDTH-1, 1100);
*/
  tft.setCursor(0, 20);
  tft.setTextSize(1);

//  tft.setFont(&fonts::Font0);
//  tft.println("TestFont0");
  tft.setFont(&fonts::Font2);
  tft.println("TestFont2");
  tft.setFont(&fonts::Font4);
  tft.println("TestFont4");
/*
  tft.setFont(&fonts::FreeSans12pt7b);
  tft.println("FreeSans12pt7b");
  tft.setFont(&fonts::FreeMono12pt7b);
  tft.println("FreeMono12pt7b");
  tft.setFont(&fonts::Font8x8C64);
  tft.println("Font8x8C64");
  tft.setFont(&fonts::Roboto_Thin_24);
  tft.println("Roboto_Thin_24");
  tft.setFont(&fonts::efontCN_12);
  tft.println("efontCN_12");
  tft.setFont(&fonts::Satisfy_24);
  tft.println("Satisfy_24");
  tft.setFont(&fonts::AsciiFont8x16);
  tft.println("AsciiFont8x16");
  tft.setFont(&fonts::AsciiFont24x48);
  tft.println("AsciiFont24x48");
*/
/*
  tft.setCursor(0, 20);
  tft.setFont(&fonts::Font2);
  tft.setTextSize(1);
  tft.println("Size 1");
  tft.setTextSize(2);
  tft.println("Size 2");
  tft.setTextSize(3);
  tft.println("Size 3");
  tft.setTextSize(3);
  tft.println("Size 3.5");
*/

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawString(F("ESP32 ROS Remote"), LCD_WIDTH/2, LCD_HEIGHT); 
}

#include <WiFi.h>
void mainScreen() {
  char buffer[64];
  IPAddress lip = WiFi.localIP();
  sprintf(buffer, FST("Local: %d.%d.%d.%d"), lip[0], lip[1], lip[2], lip[3]);   lv_obj_t *label = lv_label_create( lv_scr_act() );
   lv_label_set_text( label, buffer);
   lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );

   lv_obj_t * btn = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
    lv_obj_set_pos(btn, 20, 100);                            /*Set its position*/
    lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/

    label = lv_label_create(btn);          /*Add a label to the button*/
    lv_label_set_text(label, "Button");                     /*Set the labels text*/
    lv_obj_center(label);
}

void guiRun() {
   lv_timer_handler();
}

uint32_t count = ~0;
void display_test() {
  tft.startWrite();
  tft.setRotation(++count & 7);
  tft.setColorDepth((count & 8) ? 16 : 24);

  tft.setTextColor(TFT_WHITE);
  tft.drawNumber(tft.getRotation(), 16, 0);

  tft.setTextColor(0xFF0000U);
  tft.drawString("R", 30, 16);
  tft.setTextColor(0x00FF00U);
  tft.drawString("G", 40, 16);
  tft.setTextColor(0x0000FFU);
  tft.drawString("B", 50, 16);

  tft.drawRect(30,30,tft.width()-60,tft.height()-60,count*7);
  tft.drawFastHLine(0, 0, 10);

  tft.endWrite();

  int32_t x, y;
  if (tft.getTouch(&x, &y)) {
    tft.fillRect(x-2, y-2, 5, 5, count*7);
  }
}
#endif