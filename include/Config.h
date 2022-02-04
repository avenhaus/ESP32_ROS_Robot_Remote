#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

/********************************************\
|*  Pin Definitions
\********************************************/

/*
https://drive.google.com/file/d/1gbKM7DA7PI7s1-ne_VomcjOrb0bE2TPZ/view
---+------+----+-----+-----+-----------+---------------------------
No.| GPIO | IO | RTC | ADC | Default   | Function
---+------+----+-----+-----+-----------+---------------------------
25 |   0* | IO | R11 | 2_1 | Boot      | INPUT_CLK
35 |   1  | IO |     |     | UART0_TXD | USB Programming/Debug
24 |   2* | IO | R12 | 2_2 |           | LCD_DC
34 |   3  | IO |     |     | UART0_RXD | USB Programming/Debug
26 |   4* | IO | R10 | 2_0 |           | INPUT_LOAD
29 |   5* | IO |     |     | SPI0_SS   | SD_CS (LED)  
14 |  12* | IO | R15 | 2_5 |           | LCD_LED
16 |  13  | IO | R14 | 2_4 |           | INPUT_IN (or INPUT_CS)
13 |  14  | IO | R16 | 2_6 |           | TOUCH_CS
23 |  15* | IO | R13 | 2_3 |           | LCD_CS
27 |  16+ | IO |     |     | UART2_RXD | 
28 |  17+ | IO |     |     | UART2_TXD | 
30 |  18  | IO |     |     | SPI0_SCK  | SCK LCD,Touch,SD
31 |  19  | IO |     |     | SPI0_MISO | MISO Touch, SD
33 |  21  | IO |     |     | I2C0_SDA  | Buzzer 
36 |  22  | IO |     |     | I2C0_SCL  | 
37 |  23  | IO |     |     | SPI0_MOSI | MOSI LCD, SD
10 |  25  | IO | R06 | 2_8 |DAC1/I2S-DT| Battery
11 |  26  | IO | R07 | 2_9 |DAC2/I2S-WS| 
12 |  27  | IO | R17 | 2_7 | I2S-BCK   | 
8  |  32  | IO | R09 | 1_4 |           | Pot_R
9  |  33  | IO | R08 | 1_5 |           | R_JOY_Y
6  |  34  | I  | R04 | 1_6 |           | R_JOY_X
7  |  35  | I  | R05 | 1_7 |           | Pot_L
4  |  36  | I  | R00 | 1_0 | SENSOR_VP | L_JOY_Y
5  |  39  | I  | R03 | 1_3 | SENSOR_VN | L_JOY_X
3  |  EN  | I  |     |     | RESET     | Reset LCD       
---+------+----+-----+-----+-----------+---------------------------
(IO6 to IO11 are used by the internal FLASH and are not useable)
22 x I/O  + 4 x input only = 26 usable pins 
GPIO_34 - GPIO_39 have no internal pullup / pulldown.
- ADC2 can not be used with WIFI/BT (easily)
+ GPIO 16 and 17 are not available on WROVER (PSRAM)
* Strapping pins: IO0, IO2, IO4, IO5 (HIGH), IO12 (LOW), IO15 (HIGH)
*/

#define LCD_SCK_PIN 18
#define LCD_MOSI_PIN 23
#define LCD_MISO_PIN 19
#define LCD_RST_PIN -1
#define LCD_DC_PIN 2
#define LCD_CS_PIN 15
#define LCD_LED_PIN 12
#define LCD_LED_PWM_CHANNEL 0

#define TOUCH_CS_PIN 14
#define TOUCH_CS_IRQ -1

#define SD_CS_PIN 5
#define LED_PIN 5

#define BUZZER_PIN 21

#define INPUT_SPI_CHANNEL VSPI
#define INPUT_SPI_SPEED 10000000
#define INPUT_SCK_PIN 0
#define INPUT_LOAD_PIN 4 // 74HC165 HIGH during shifting
#define INPUT_MISO_PIN 13

#define ADC_VREF 1100

#define BATTERY_PIN -1
#define R_POT1_PIN 34
#define L_POT1_PIN 33

#define R_JOY_X_PIN 36 
#define R_JOY_Y_PIN 39 
#define R_JOY_R_PIN -1

#define L_JOY_X_PIN 35 
#define L_JOY_Y_PIN 32 
#define L_JOY_R_PIN -1

/* ============================================== *\
 * Extended Inputs (from shift registers 74HC165)
\* ============================================== */

#define LEFT_ENCODER1_A_BIT 22
#define LEFT_ENCODER1_B_BIT 23
#define LEFT_ENCODER1_BUTTON_BIT 21

#define RIGHT_ENCODER1_A_BIT 18
#define RIGHT_ENCODER1_B_BIT 19
#define RIGHT_ENCODER1_BUTTON_BIT 20

#define LEFT_BUTTON_JOY_BIT 17
#define LEFT_BUTTON_I1_BIT 28 
#define LEFT_BUTTON_I2_BIT 29
#define LEFT_BUTTON_T1_BIT 30
#define LEFT_BUTTON_T2_BIT 31
#define LEFT_BUTTON_SW1_BIT 10
#define LEFT_BUTTON_SW2_BIT 11
#define LEFT_BUTTON_DSW1A_BIT 24
#define LEFT_BUTTON_DSW1B_BIT 25

#define RIGHT_BUTTON_JOY_BIT 16
#define RIGHT_BUTTON_I1_BIT 12 
#define RIGHT_BUTTON_I2_BIT 13
#define RIGHT_BUTTON_T1_BIT 14
#define RIGHT_BUTTON_T2_BIT 15
#define RIGHT_BUTTON_SW1_BIT 26
#define RIGHT_BUTTON_SW2_BIT 27
#define RIGHT_BUTTON_DSW1A_BIT 9
#define RIGHT_BUTTON_DSW1B_BIT 8


#define EX_INPUT(BIT) (BIT < 0 ? 0 : (extended_inputs >> BIT) & 1)

/* ============================================== *\
 * Debug
\* ============================================== */

#define SERIAL_DEBUG 1
#define SERIAL_SPEED 115200

#if SERIAL_DEBUG < 1
#define DEBUG_println(...) 
#define DEBUG_print(...) 
#define DEBUG_printf(...) 
#else
#define DEBUG_println(...) if (debugStream) {debugStream->println(__VA_ARGS__);}
#define DEBUG_print(...) if (debugStream) {debugStream->print(__VA_ARGS__);}
#define DEBUG_printf(...) if (debugStream) {debugStream->printf(__VA_ARGS__);}
#endif

#define FST (const char *)F
#define PM (const __FlashStringHelper *)

extern Print* debugStream;

/* ============================================== *\
 * Constants
\* ============================================== */


#define COMMAND_BUFFER_SIZE 64

#define ENABLE_OTA 1
#define ENABLE_TELNET 1
#define ENABLE_WEB_SERVER 1
#define WEBUI_TABS "control,settings,files,wizard"
#define WEBUI_START_TAB "settings"
#define ENABLE_EMBEDDED_WEB_UI 1
#define ENABLE_SPIFFS_WEB_UI 1
#define ENABLE_SPIFFS_WEB_CONTENT 0
#define ENABLE_MDNS 1
#define ENABLE_NTP 1
#define ENABLE_SPIFFS 1

#define ENABLE_STATE_REG 1
#define ENABLE_WS_STATE_MONITOR 1
// #define USE_NETWORK_TASK 1

#define ENABLE_DISPLAY 1

// PST8PDT,M3.2.0,M11.1.0
#define TIME_ZONE "PST8PDT,M3.2.0/2:00:00,M11.1.0/2:00:00"
#define TIME_FORMAT "%H:%M:%S"
#define DATE_FORMAT "%d.%m.%Y"

// "time.nist.gov"
#define NTP_SERVER_1 "132.163.96.1"
// "time.windows.com"
#define NTP_SERVER_2 "13.65.245.138"


#define ROS1_HOST "192.168.0.155"
#define ROS1_PORT 11411

#define HOSTNAME "ROS-remote"
#define HTTP_PORT 80
#define TELNET_PORT 23

const char PROJECT_NAME[] PROGMEM = "ROS Remote";
const char VERSION_NUMBER[] PROGMEM = "0.2";
const char VERSION_DATE[] PROGMEM = __DATE__;
const char VERSION_TIME[] PROGMEM = __TIME__;

typedef enum JoyAxis {
    L_JOY_AXIS_X,
    L_JOY_AXIS_Y,
    L_JOY_AXIS_R,
    L_JOY_AXIS_P,
    R_JOY_AXIS_X,
    R_JOY_AXIS_Y,
    R_JOY_AXIS_R,
    R_JOY_AXIS_P,
    JOY_AXIS_SIZE
} JoyAxis;
extern float joyAxes[JOY_AXIS_SIZE];

typedef enum JoyButton {
    L_JOY_BUTTON_JOY,
    L_JOY_BUTTON_I1,
    L_JOY_BUTTON_I2,
    L_JOY_BUTTON_T1,
    L_JOY_BUTTON_T2,
    L_JOY_BUTTON_SW1,
    L_JOY_BUTTON_SW2,
    L_JOY_BUTTON_DSW1,
    L_JOY_BUTTON_ENC,
    L_JOY_BUTTON_ENCB,
    R_JOY_BUTTON_JOY,
    R_JOY_BUTTON_I1,
    R_JOY_BUTTON_I2,
    R_JOY_BUTTON_T1,
    R_JOY_BUTTON_T2,
    R_JOY_BUTTON_SW1,
    R_JOY_BUTTON_SW2,
    R_JOY_BUTTON_DSW1,
    R_JOY_BUTTON_ENC,
    R_JOY_BUTTON_ENCB,
    JOY_BUTTON_SIZE
} JoyButton;
extern int32_t joyButtons[JOY_BUTTON_SIZE];


typedef enum ErrorCode {
    EC_OK,
    EC_ERROR=500,
    EC_UNKNOWN,
    EC_TIMEOUT,
    EC_BAD_ARGUMENT,
    EC_HTTP_OK=200,
    EC_BAD_REQUEST=400,
    EC_UNAUTHORIZED=401,
    EC_PAYMENT_REQUIRED=402,
    EC_FORBIDDEN=403,
    EC_NOT_FOUND=404,
    EC_METHOD_NOT_ALLOWED=405,
    EC_NOT_ACCEPTABLE=406,
    EC_PROXY_AUTHENTICATION_REQUIRED=407,
    EC_REQUEST_TIMEOUT=408,
    EC_CONFLICT=409,
    EC_GONE=410,
    EC_LENGTH_REQUIRED=411,
    EC_PRECONDITION_FAILED=412,
    EC_PAYLOAD_TOO_LARGE=413,
    EC_URI_TOO_LONG=414,
    EC_UNSUPPORTED_MEDIA=415,
    EC_RANGE_NOT_SATISFIABLE=416,
    EC_EXPECTATION_FAILED=417,
    EC_MISDIRECTED_REQUEST=421,
    EC_UNPROCESSABLE_ENTITY=422,
    EC_LOCKED=423,
    EC_FAILED_DEPENDENCY=424,
    EC_TOO_EARLY=425,
    EC_UPGRADE_REQUIRED=426,
    EC_PRECONDITION_REQUIRED=428,
    EC_TOO_MANY_REQUESTS=429,
    EC_REQUEST_HEADER_FIELDS_TOO_LARGE=431,
    EC_UNAVAILABLE_FOR_LEGAL_REASONS=451,
    EC_INTERNAL_SERVER_ERROR=500,
    EC_NOT_IMPLEMENTED=501,
    EC_BAD_GATEWAY=502,
    EC_SERVICE_UNAVAILABLE=503,
    EC_GATEWAY_TIMEOUT=504,
    EC_HTTP_VERSION_NOT_SUPPORTED=505,
    EC_VARIANT_ALSO_NEGOTIATES=506,
    EC_INSUFFICIENT_STORAGE=507,
    EC_LOOP_DETECTED=508,
    EC_NOT_EXTENDED=510,
    EC_NETWORK_AUTHENTICATION_REQUIRED=511
} ErrorCode;

#endif
