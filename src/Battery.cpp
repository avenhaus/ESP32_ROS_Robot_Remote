/*
  MIT License
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#include "Arduino.h"
#include "Config.h"
#include "Battery.h"
#include "Analog.h"
#include "ConfigReg.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"

int batteryRaw = 0;
float batteryVoltage = 0.0;
float batteryVoltageFiltered = 0.0;
int batteryChargeLevel = 0;
uint32_t batteryReadTs_ = 0;

ConfigUInt32 configAdcVref(FST("ADC VRef"), ADC_VREF, FST("ADC calibration in mV"));

//Characterize ADC at particular atten
esp_adc_cal_characteristics_t adc1Chars;
esp_adc_cal_characteristics_t adc2Chars;

// Table and binary search adapted from: https://github.com/pangodream/18650CL
float LION_VOLTS_TABLE[] PROGMEM = {
    3.200, 
    3.250, 3.300, 3.350, 3.400, 3.450, 3.500, 3.550, 3.600, 3.650, 3.700, 3.703, 3.706, 3.710, 3.713, 3.716, 
    3.719, 3.723, 3.726, 3.729, 3.732, 3.735, 3.739, 3.742, 3.745, 3.748, 3.752, 3.755, 3.758, 3.761, 3.765, 
    3.768, 3.771, 3.774, 3.777, 3.781, 3.784, 3.787, 3.790, 3.794, 3.797, 3.800, 3.805, 3.811, 3.816, 3.821, 
    3.826, 3.832, 3.837, 3.842, 3.847, 3.853, 3.858, 3.863, 3.868, 3.874, 3.879, 3.884, 3.889, 3.895, 3.900, 
    3.906, 3.911, 3.917, 3.922, 3.928, 3.933, 3.939, 3.944, 3.950, 3.956, 3.961, 3.967, 3.972, 3.978, 3.983, 
    3.989, 3.994, 4.000, 4.008, 4.015, 4.023, 4.031, 4.038, 4.046, 4.054, 4.062, 4.069, 4.077, 4.085, 4.092, 
    4.100, 4.111, 4.122, 4.133, 4.144, 4.156, 4.167, 4.178, 4.189, 4.200
};

int getLionCellChargeLevel(float volts) {
  int idx = 50;
  int prev = 0;
  int half = 0;
  if (volts >= 4.2) { return 100; }
  if (volts <= 3.2) { return 0; }
  while(true) {
    half = abs(idx - prev) / 2;
    prev = idx;
    if(volts >= LION_VOLTS_TABLE[idx]) { idx = idx + half; }
    else { idx = idx - half; }
    if (prev == idx) { break; }
  }
  return idx;
}


void adcInit() {
    esp_adc_cal_value_t valType = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, configAdcVref.get(), &adc1Chars);
    //Check type of calibration value used to characterize ADC
    if (valType == ESP_ADC_CAL_VAL_EFUSE_VREF) { DEBUG_println(FST("eFuse ADC VRef")); } 
    else if (valType == ESP_ADC_CAL_VAL_EFUSE_TP) { DEBUG_println(FST("Two Point ADC VRef")); } 
    else { DEBUG_printf(FST("Default ADC VRef: %3f\n"), configAdcVref.get() * 0.001); }

    // esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, configAdcVref.get(), &adc2Chars);

}

void batteryRun(uint32_t now/*=0*/) {
  if (now == 0) { now = millis(); }
  if (now < batteryReadTs_ + BATTERY_READ_MS) { return; }
  batteryReadTs_ = now;
  batteryRaw = safeAnalogRead(BATTERY_PIN); 
  float pinVoltage = esp_adc_cal_raw_to_voltage(batteryRaw, &adc1Chars) * 0.001;
  batteryVoltage = pinVoltage * BATTERY_CONV_FACTOR;
  if (batteryVoltageFiltered == 0.0) { batteryVoltageFiltered = batteryVoltage; }
  else { batteryVoltageFiltered = batteryVoltageFiltered * 0.9 + batteryVoltage * 0.1; }
  batteryChargeLevel = getLionCellChargeLevel(batteryVoltageFiltered / BATTERY_CELLS);
  DEBUG_printf(FST("Battery: %d  %.3fV  %.3fV  %.3fV  %d%%\n"), batteryRaw, pinVoltage, batteryVoltage, batteryVoltageFiltered, batteryChargeLevel);
  return;
}