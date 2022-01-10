#include <WiFi.h>
#include <EEPROM.h>

#include "Config.h"
#include "ConfVar.h"
#include "Secret.h"


ConfigGroup* ConfigGroup::mainGroup = nullptr;
//ConfigUInt32 configMagic(FST("MAGIC"), CONFIG_MAGIC, FST("%0X"), FST("Hidden config ID"), nullptr, nullptr, nullptr, nullptr, 0xFF);

size_t ConfigGroup::jsonName(char* buffer, size_t size, const char* name) {
  size_t n = 0;
  if (n < size-1) { buffer[n++] = '"'; }
  while (n < size-1 && *name) {
    char c = *name++;
    if (c == ' ') { c = '_'; }
    buffer[n] = c;    
    n++;
  }
  if (n < size-1) { buffer[n++] = '"'; }
  if (n < size-1) { buffer[n++] = ':'; }
  buffer[n] = '\0';
  return n;
}

bool ConfigGroup::matchesJsonName(const char* jName, const char* name) {
  while (*jName && *name) {
    if (*jName != *name) {  
      if (*jName != '_' || *name != ' ') { return false; }
    }
    jName++; 
    name++;
  }
  if (*jName != '\0' || *name != '\0') { return false; }
  return true;
}

size_t ConfigGroup::toJsonStr(char* buffer, size_t size, bool noName) {
    size_t n = 0;
    if (!noName) { n += ConfigGroup::jsonName(buffer+n, size-n, name_); }
    if (n < size-1) { buffer[n++] = '{'; }
    bool first = true;
    for(auto g: children_) {
      if (!first && (n < size-1)) { buffer[n++] = ','; }
      first = false; 
      n += g->toJsonStr(buffer+n, size-n);
    }
    for(auto v: vars_) {
      if (!first && (n < size-1)) { buffer[n++] = ','; }
      first = false; 
      n += v->toJsonStr(buffer+n, size-n);
    }
    if (n < size-1) { buffer[n++] = '}'; }
    buffer[n] = '\0';
    return n;
}

ConfigGroup* ConfigGroup::findChild(const char* name) {
    for(auto g: children_) {
      if (matchesJsonName(name, g->name())) return g;
    }
    return nullptr;
}

ConfigVar* ConfigGroup::findVar(const char* name) {
    for(auto v: vars_) {
      if (matchesJsonName(name, v->name())) return v;
    }
    return nullptr;
}

bool ConfigGroup::fromJson(const JsonObject& obj) {
  bool err = false;
  // Loop through all the key-value pairs in obj
  for (JsonPair p : obj) {
    ConfigGroup* child = findChild(p.key().c_str());
    if (child) { err |= child->fromJson(p.value()); }
    else {
      ConfigVar* var = findVar(p.key().c_str());
      if (var) { err |= var->fromJson(p.value()); }
      else { 
        DEBUG_printf(FST("Could not find JSON config \"%s\"\n"), p.key().c_str());
        err = true; 
      }
    }
  }
  return err;
}

uint32_t calculateConfigCrc(const char* data, size_t size) {
  uint32_t* ptr = (uint32_t*) data;
  uint32_t crc = ~size;
  size_t n = 0;
  while (n <= (size-4)) {
    crc += *ptr++;
    n += 4;
  }
  while (n < size) {
    crc += data[n++];
  }
  return crc;
}


/*
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
*/

bool parseConfigJson(char* jsonStr) {
  StaticJsonDocument<1024> doc; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  DeserializationError err = deserializeJson(doc, jsonStr);
  if (err != DeserializationError::Ok) {
    DEBUG_printf(FST("JSON config parsing failed: %s\n"), (const char *)err.f_str());
    return true;
  }
  JsonObject obj = doc.as<JsonObject>();
  return ConfigGroup::mainGroup->fromJson(obj);
}


bool loadConfig() {
  ConfigHeader header;
  char buffer[CONFIG_BUFFER_SIZE];
  EEPROM.begin(sizeof(ConfigHeader) + CONFIG_BUFFER_SIZE);
  bool ok = true;
  for (int i=0; i<sizeof(ConfigHeader); i++) { ((uint8_t*)&header)[i] = EEPROM.read(i); }
  if (header.magic != CONFIG_MAGIC) {
    DEBUG_print(F("Bad EEPROM magic: ")); 
    DEBUG_println(header.magic, HEX); 
    ok = false;
  }
  DEBUG_printf(FST("EEPROM config size: %d  Buffer Size: %d\n"), header.size, CONFIG_BUFFER_SIZE); 
  if (header.size > CONFIG_BUFFER_SIZE-1) {
    DEBUG_print(F("Bad EEPROM config size: ")); 
    DEBUG_println(header.size); 
    ok = false;
  }
  for (int i=0; i<header.size; i++) { buffer[i] = EEPROM.read(i+sizeof(ConfigHeader)); }
  buffer[header.size] = '\0';
  uint32_t crc = calculateConfigCrc(buffer, header.size);
  if (crc != header.crc) {
    DEBUG_printf(FST("Bad EEPROM data crc: %0X vs header crc: %0X\n"), crc, header.crc); 
    ok = false;
  }
  if (ok) {
    DEBUG_println(F("Found valid EEPROM config data.")); 
    DEBUG_println(buffer);
  }
  EEPROM.end();
  yield();
  if (ok) { ok |= !parseConfigJson(buffer); }
  return ok;
}

void saveConfig() {
  char buffer[CONFIG_BUFFER_SIZE];
  size_t size = ConfigGroup::mainGroup->toJsonStr(buffer, sizeof(buffer), true);
  if (size >= CONFIG_BUFFER_SIZE-1) {
    DEBUG_printf(FST("!!! Not enough buffer space (%d) to save config to EEPROM !!!\n"), CONFIG_BUFFER_SIZE);
    return;
  }
  uint32_t crc = calculateConfigCrc(buffer, size);
  DEBUG_printf(FST("Saving config of size %d to EEPROM. Buffer space:%d CRC:%0X\n"), size, CONFIG_BUFFER_SIZE, crc);

  ConfigHeader header = {
    CONFIG_MAGIC,
    size,
    crc
  };

  EEPROM.begin(sizeof(ConfigHeader) + size);
  for (size_t i=0; i<sizeof(ConfigHeader); i++) { EEPROM.write(i, ((uint8_t*)&header)[i]); }
  for (size_t i=0; i<size; i++) { EEPROM.write(i+sizeof(ConfigHeader), buffer[i]); }
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

