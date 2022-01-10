/*======================================================================*\
 * ESP32 ROS Remote Controller
\*======================================================================*/

#include <Arduino.h>

#include "Config.h"
#include "ConfVar.h"
#include "SPIFFS.h"
#include "Display.h"
#include "ROS1.h"
#include "Analog.h"
#include "Battery.h"
#include "Encoder.h"

void extendedInputSetup();
uint32_t getExtendedInputs();
extern uint32_t extended_inputs;
extern uint32_t old_extended_inputs;

Stream* debugStream = &Serial;

Encoder encoderLeft;
Encoder encoderRight;

Joystick leftJoyX(L_JOY_X_PIN);
Joystick leftJoyY(L_JOY_Y_PIN);
Joystick leftJoyR(L_JOY_R_PIN);
Potentiometer leftPot1(L_POT1_PIN);

Joystick rightJoyX(R_JOY_X_PIN);
Joystick rightJoyY(R_JOY_Y_PIN);
Joystick rightJoyR(R_JOY_R_PIN);
Potentiometer rightPot1(R_POT1_PIN);

uint32_t analogReadTs = 0;

float joyAxes[JOY_AXIS_SIZE] = {0};
int32_t joyButtons[JOY_BUTTON_SIZE] = {0};

uint32_t inputSameCount = 0;

void networkInit(void);
void networkRun(void);

void setup() {
  Serial.begin(SERIAL_SPEED);
  DEBUG_printf(FST("\n\nROS Remote %s | %s | %s\n"), version_number, version_date, version_time);
  DEBUG_printf(FST("Compiled with ESP32 SDK:%s\n\n"), ESP.getSdkVersion());

  adcInit();
  adc2RegSave(); // Save ADC2 registers before WiFi

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  

  loadConfig();

  DEBUG_println("========= Confi===");
  char buffer[2048];
  DEBUG_println(ConfigGroup::mainGroup->toJsonStr(buffer, sizeof(buffer), true));
  DEBUG_println(buffer);


  // This must be executed before Display initialization.
  // Otherwise SPI gets messed up for some reason.
  extendedInputSetup();

  #if ENABLE_DISPLAY
  displaySetup();
  displayBootScreen();  
  DEBUG_print(F("Screen Done\n"));
  #endif
  

#if  ENABLE_WEB_SERVER
  if(!SPIFFS.begin(true)){
    DEBUG_println(FST("An Error has occurred while mounting SPIFFS"));
    return;
  }
#endif

  networkInit();
  rosInit();

  #if ENABLE_DISPLAY
  mainScreen();
  #endif
  //digitalWrite(LED_PIN, HIGH);  
}

void loop() {
  #if !USE_NETWORK_TASK
  networkRun();
  #endif
 
  getExtendedInputs();  
  if (extended_inputs != old_extended_inputs) { inputSameCount = 0; }
  else { inputSameCount++; }
  if (inputSameCount == 5) { // Simple debounce
  
    joyButtons[L_JOY_BUTTON_ENC] = encoderLeft.update((extended_inputs >> LEFT_ENCODER1_A_BIT) & 3);
    joyButtons[L_JOY_BUTTON_ENCB] = !EX_INPUT(LEFT_ENCODER1_BUTTON_BIT);
    joyButtons[R_JOY_BUTTON_ENC] = encoderRight.update((extended_inputs >> RIGHT_ENCODER1_A_BIT) & 3);
    joyButtons[R_JOY_BUTTON_ENCB] = !EX_INPUT(RIGHT_ENCODER1_BUTTON_BIT);

    DEBUG_printf(FST("Inputs: %08X  L:%d  R:%d \n"), ~extended_inputs, encoderLeft.counter, encoderRight.counter);

    joyButtons[L_JOY_BUTTON_JOY] = !EX_INPUT(LEFT_BUTTON_JOY_BIT);
    joyButtons[L_JOY_BUTTON_I1] = !EX_INPUT(LEFT_BUTTON_I1_BIT);
    joyButtons[L_JOY_BUTTON_I2] = !EX_INPUT(LEFT_BUTTON_I2_BIT);
    joyButtons[L_JOY_BUTTON_T1] = !EX_INPUT(LEFT_BUTTON_T1_BIT);
    joyButtons[L_JOY_BUTTON_T2] = !EX_INPUT(LEFT_BUTTON_T2_BIT);
    joyButtons[L_JOY_BUTTON_SW1] = !EX_INPUT(LEFT_BUTTON_SW1_BIT);
    joyButtons[L_JOY_BUTTON_SW2] = !EX_INPUT(LEFT_BUTTON_SW2_BIT);
    joyButtons[L_JOY_BUTTON_DSW1] = !EX_INPUT(LEFT_BUTTON_DSW1A_BIT) ? 1 : !EX_INPUT(LEFT_BUTTON_DSW1B_BIT) ? -1 : 0;

    joyButtons[R_JOY_BUTTON_JOY] = !EX_INPUT(RIGHT_BUTTON_JOY_BIT);
    joyButtons[R_JOY_BUTTON_I1] = !EX_INPUT(RIGHT_BUTTON_I1_BIT);
    joyButtons[R_JOY_BUTTON_I2] = !EX_INPUT(RIGHT_BUTTON_I2_BIT);
    joyButtons[R_JOY_BUTTON_T1] = !EX_INPUT(RIGHT_BUTTON_T1_BIT);
    joyButtons[R_JOY_BUTTON_T2] = !EX_INPUT(RIGHT_BUTTON_T2_BIT);
    joyButtons[R_JOY_BUTTON_SW1] = !EX_INPUT(RIGHT_BUTTON_SW1_BIT);
    joyButtons[R_JOY_BUTTON_SW2] = !EX_INPUT(RIGHT_BUTTON_SW2_BIT);
    joyButtons[R_JOY_BUTTON_DSW1] = !EX_INPUT(RIGHT_BUTTON_DSW1A_BIT) ? 1 : !EX_INPUT(RIGHT_BUTTON_DSW1B_BIT) ? -1 : 0;

    //DEBUG_printf(FST("Inputs: %04X %d\n"), extended_inputs, joyButtons[L_JOY_BUTTON_ENC]);
  }

  uint32_t now = millis();
  if (now - analogReadTs >= 2) {
    analogReadTs = now;
    joyAxes[L_JOY_AXIS_X] = leftJoyX.read();
    joyAxes[L_JOY_AXIS_Y] = leftJoyY.read();
    joyAxes[L_JOY_AXIS_R] = leftJoyR.read();
    joyAxes[L_JOY_AXIS_P] = leftPot1.read();

    joyAxes[R_JOY_AXIS_X] = rightJoyX.read();
    joyAxes[R_JOY_AXIS_Y] = rightJoyY.read();
    joyAxes[R_JOY_AXIS_R] = rightJoyR.read();
    joyAxes[R_JOY_AXIS_P] = rightPot1.read();
    //DEBUG_printf(FST("Analog: LX %d  %.3f %.3f  LP %d  %.3f %.3f  LY %d   RX %d  RY %d  RP %d\n"), leftJoyX.raw, leftJoyX.value, leftJoyX.fvalue, leftPot1.raw, leftPot1.value, leftPot1.fvalue, leftJoyY.raw, rightJoyX.raw, rightJoyY.raw, rightPot1.raw);
    //DEBUG_printf(FST("Analog: LY %d  %.3f %.3f \n"), leftJoyY.raw, leftJoyY.value, leftJoyY.fvalue);
    //DEBUG_printf(FST("Analog: LX %4d  %.3f | LY %4d  %.3f | LP %4d  %.3f || RX %4d  %.3f | RY %4d  %.3f | RP %4d  %.3f\n"), leftJoyX.raw, leftJoyX.fvalue, leftJoyY.raw, leftJoyY.fvalue, leftPot1.raw, leftPot1.fvalue, rightJoyX.raw, rightJoyX.fvalue, rightJoyY.raw, rightJoyY.fvalue, rightPot1.raw, rightPot1.fvalue);

  }

  #if ENABLE_DISPLAY
  guiRun();
  #endif

  #if BATTERY_PIN >= 0 
  batteryRun(now);
  if (batteryChargeLevel <= 2) {
    //  DEBUG_println(FST("Battery is low!"));
    // shutdown();
  }
  #endif

  //vTaskDelay(100);

}