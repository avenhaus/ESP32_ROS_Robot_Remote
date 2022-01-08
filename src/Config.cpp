#include <WiFi.h>

#include "Config.h"
#include "Secret.h"
#include <EEPROM.h>

ConfigData config = {
  CONFIG_MAGIC, // magic
  sizeof(ConfigData), //configSize
  90,              // displayBrightness
  {0,0,0,0,0,0},
  "%H:%M:%S",
  "%d.%m.%Y",
  TIME_ZONE,
  ADC_VREF, // adcVref
  { // wifi
    WIFI_SSID, // ssid
    WIFI_PASSWORD, // password
    HOSTNAME,
    {0,0,0,0}, // ip
    {0,0,0,0}, // dns
    {0,0,0,0}, // gateway
    {0,0,0,0}, // subnet   
    false      // isDisabled 
  },
  0 // CRC
};


uint32_t calculateConfigCrc(const ConfigData& data) {
  uint32_t* ptr = (uint32_t*) &data;
  uint32_t crc = 0;
  for (int i=0; i<(sizeof(ConfigData)/sizeof(uint32_t)-1); i++) { crc += *ptr++; }
  return crc;
}

bool loadConfig() {
  EEPROM.begin(sizeof(ConfigData));
  ConfigData data;
  bool ok = true;
  for (int i=0; i<sizeof(ConfigData); i++) { ((uint8_t*)&data)[i] = EEPROM.read(i); }
  if (data.magic != CONFIG_MAGIC) {
    DEBUG_print(F("Bad EEPROM magic: ")); 
    DEBUG_println(data.magic, HEX); 
    ok = false;
  }
  DEBUG_printf(FST("EEPROM config size: %d vs %d\n"), data.configSize, sizeof(ConfigData)); 
  if (data.configSize != sizeof(ConfigData)) {
    DEBUG_print(F("Bad EEPROM config size: ")); 
    DEBUG_println(data.configSize); 
    ok = false;
  }
  uint32_t crc = calculateConfigCrc(data);
  if (crc != data.crc) {
    DEBUG_print(F("Bad EEPROM crc: ")); 
    DEBUG_println(crc, HEX); 
    ok = false;
  }
  if (ok) {
    DEBUG_println(F("Found valid EEPROM config data.")); 
    memcpy(&config, &data, sizeof(ConfigData));
  }
  EEPROM.end();
  yield();
  return ok;
}

void saveConfig() {
  EEPROM.begin(sizeof(ConfigData));
  config.magic = CONFIG_MAGIC;
  config.configSize = sizeof(ConfigData);
  int32_t crc = 0;
  uint8_t* p = (uint8_t*)&config;
  for (int i=0; i<(sizeof(ConfigData) - sizeof(ConfigData::crc)); i++) { crc += *p++; }
  config.crc = calculateConfigCrc(config);
  for (int i=0; i<sizeof(ConfigData); i++) { EEPROM.write(i, ((uint8_t*)&config)[i]); }
  EEPROM.end();
  yield();
  DEBUG_println(F("Wrote config data to EEPROM"));
}


bool parseIpAddr(uint8_t ip[4], const char* str) {
  int i = 0;
  while(*str && i<4) {
    ip[i++] = atoi(str);
    while(*str == ' ' || (*str >= '0' && *str <= '9')) { str++; }
    if (*str == 0) { break; }
    if (*str == '.') { str++; }
    else {
      DEBUG_print(F("Bad IP address caracter: ")); 
      DEBUG_println(*str); 
      return true;
    }
  }
  return false;
}

#define SET_STRING_FIELD(NAME, FIELD) if (strcmp_P(FST(NAME), field) == 0) { strncpy(FIELD, data, sizeof(FIELD)-1); }
#define SET_FLOAT_FIELD(NAME, FIELD) if (strcmp_P(FST(NAME), field) == 0) { FIELD = atof(data); }
#define SET_INT_FIELD(NAME, FIELD) if (strcmp_P(FST(NAME), field) == 0) { FIELD = atoi(data); }
#define SET_IP_FIELD(NAME, FIELD) if (strcmp_P(FST(NAME), field) == 0) { parseIpAddr(FIELD, data); }

void setConfigData(const char* field, const char* data) {
  SET_STRING_FIELD("w.ssid", config.wifi.ssid)
  else SET_STRING_FIELD("w.password", config.wifi.password)
  else SET_IP_FIELD("w.ip", config.wifi.ip)
  else SET_IP_FIELD("w.dns", config.wifi.dns)
  else SET_IP_FIELD("w.gateway", config.wifi.gateway)
  else SET_IP_FIELD("w.subnet", config.wifi.subnet)
  else SET_STRING_FIELD("tz", config.timeZone)
  else SET_INT_FIELD("adcVref", config.adcVref)
  else { 
    DEBUG_print(F("Unknown Form Field: ")); 
    DEBUG_print(field); 
    DEBUG_print(F(" = ")); 
    DEBUG_println(data); 
  }
}

#define FORM_FIELD(LABEL, NAME, ...) { \
  client.print(F(LABEL ": <input type='text' name='" NAME "' value='")); \
  client.print(__VA_ARGS__); \
  client.print(F("'><br>\n")); }

#define FORM_IP_FIELD(LABEL, NAME, DATA) { \
  client.print(F(LABEL ": <input type='text' name='" NAME "' value='")); \
  client.printf(FST("%d.%d.%d.%d"), DATA[0], DATA[1], DATA[2], DATA[3]); \
  client.print(F("'><br>\n")); }

void buildConfigForm(WiFiClient &client) {
  client.print(F("<h1>Configuration</h1>\n"));
  client.print(F("<form method='post'>\n"));

  client.print(F("<h2>WIFI</h2>\n"));
  FORM_FIELD("SSID", "w.ssid", config.wifi.ssid);
  FORM_FIELD("Password", "w.password", config.wifi.password);
  FORM_IP_FIELD("IP", "w.ip", config.wifi.ip);
  FORM_IP_FIELD("DNS Server", "w.dns", config.wifi.dns);
  FORM_IP_FIELD("Gateway", "w.gateway", config.wifi.gateway);
  FORM_IP_FIELD("Subnet", "w.subnet", config.wifi.subnet);


  client.print(F("<h2>Time Zone</h2>\n"));
  FORM_FIELD("Time Zone", "tz", config.timeZone);

  client.print(F("<h2>Calibration</h2>\n"));
  FORM_FIELD("ADC", "adc_calibration", config.adcVref, 12);

  client.print(F("<input type='submit' value='Submit'></form>\n"));
}
