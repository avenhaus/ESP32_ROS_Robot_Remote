/*======================================================================*\
|* Configuration Variable Registry
|*
|* This framework provides a hierarchical registry for configuration 
|* variables. Each variable can register defaults, formatting, references,
|* getter/setter callbacks etc.
|*
|* The variables can be persisted in EEPROM (as JSON). The registry
|* can be used to control or read variables from CLI, Web Interfaces
|* or Display GUI interfaces.
\*======================================================================*/

#include <WiFi.h>
#include <EEPROM.h>

#include "Config.h"
#include "ConfigReg.h"
#include "Secret.h"

ConfigGroup* ConfigGroup::mainGroup = nullptr;

/************************************************************************\
|* Global Functions
\************************************************************************/

void saveConfig() {
  char buffer[CONFIG_BUFFER_SIZE];
  size_t size = ConfigGroup::mainGroup->toJsonStr(buffer, sizeof(buffer), true);
  if (size >= CONFIG_BUFFER_SIZE-1) {
    DEBUG_printf(FST("!!! Not enough buffer space (%d) to save config to EEPROM !!!\n"), CONFIG_BUFFER_SIZE);
    return;
  }
  uint32_t crc = StrTool::calculateCrc(buffer, size);
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
  uint32_t crc = StrTool::calculateCrc(buffer, header.size);
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

bool parseConfigJson(char* jsonStr) {
  StaticJsonDocument<CONFIG_BUFFER_SIZE> doc; // !!!!! TODO:: check best size !!!!!!
  DeserializationError err = deserializeJson(doc, jsonStr);
  if (err != DeserializationError::Ok) {
    DEBUG_printf(FST("JSON config parsing failed: %s\n"), (const char *)err.f_str());
    return true;
  }
  JsonObject obj = doc.as<JsonObject>();
  return ConfigGroup::mainGroup->setFromJson(obj);
}


void defaultConfig() {
  if (ConfigGroup::mainGroup) { ConfigGroup::mainGroup->setDefaults(); }
}


/************************************************************************\
|* Config Group
\************************************************************************/

size_t ConfigGroup::toJsonStr(char* buffer, size_t size, bool noName) {
    size_t n = 0;
    if (!noName) { n += StrTool::toJsonName(buffer+n, size-n, name_); }
    if (n < size-1) { buffer[n++] = '{'; }
    bool first = true;
    for(auto v: vars_) {
      if (!first && (n < size-1)) { buffer[n++] = ','; }
      first = false; 
      n += v->toJsonStr(buffer+n, size-n);
    }
    for(auto g: children_) {
      if (!first && (n < size-1)) { buffer[n++] = ','; }
      first = false; 
      n += g->toJsonStr(buffer+n, size-n);
    }
    if (n < size-1) { buffer[n++] = '}'; }
    buffer[n] = '\0';
    return n;
}

ConfigGroup* ConfigGroup::findChild(const char* name) {
    for(auto g: children_) {
      if (StrTool::matchesVarName(name, g->name())) { return g; }
    }
    return nullptr;
}

ConfigVar* ConfigGroup::findVar(const char* name) {
    for(auto v: vars_) {
      if (StrTool::matchesVarName(name, v->name())) { return v; }
    }
    return nullptr;
}

ConfigVar* ConfigGroup::findVarByFullName(const char* name, bool matchCase/*=true*/) {
    size_t n = 0;
    for(auto v: vars_) {
      n = StrTool::matchesNamePart(name, v->name(), matchCase);
      if (n && name[n] != '.') { return v; }
    }
    for(auto g: children_) {
      n = StrTool::matchesNamePart(name, g->name(), matchCase);
      if (n && name[n] == '.') { return g->findVarByFullName(name + n + 1, matchCase); }
    }
    return nullptr;
}

ConfigVar* ConfigGroup::get(size_t n) {
    if (n >= varCount_) { return nullptr; }
    if (n < vars_.size()) { return vars_.at(n); }
    n -= vars_.size();
    for(auto g: children_) {
      if (n < g->size()) { return g->get(n); }
      n -= g->size();
    }
    return nullptr;
}

size_t ConfigGroup::getVarName(char* buffer, size_t size, size_t index) {

    if (index >= varCount_) { return 0; }
    if (index < vars_.size()) { 
      return StrTool::toVarName(buffer, size, vars_.at(index)->name()); 
    }
    index -= vars_.size();
    for(auto g: children_) {
      if (index < g->size()) { 
        size_t n = StrTool::toVarName(buffer, size, g->name_);
        if (n < size -1) { buffer[n++] = '.'; }
        buffer[n] = '\0';
        return g->getVarName(buffer+n, size-n, index); 
      }
      index -= g->size();
    }
    return 0;
}


std::vector<ConfigVar*>::iterator ConfigGroup::getIt(size_t n) {
    if (n >= varCount_) { return vars_.end(); }
    if (n <= vars_.size()) { return vars_.begin() + n; }
    n -= vars_.size();
    for(auto g: children_) {
      if (n <= g->size()) { return g->getIt(n); }
      n -= g->size();
    }
    return vars_.end();
}


bool ConfigGroup::setFromJson(const JsonObject& obj) {
  bool err = false;
  // Loop through all the key-value pairs in obj
  for (JsonPair p : obj) {
    ConfigGroup* child = findChild(p.key().c_str());
    if (child) { err |= child->setFromJson(p.value()); }
    else {
      ConfigVar* var = findVar(p.key().c_str());
      if (var) { err |= var->setFromJson(p.value()); }
      else { 
        DEBUG_printf(FST("Could not find JSON config \"%s\"\n"), p.key().c_str());
        err = true; 
      }
    }
  }
  return err;
}

void ConfigGroup::setDefaults() {
    for(auto v: vars_) { v->setDefault(); }
    for(auto g: children_) { g->setDefaults(); }
}


/************************************************************************\
|* String Helper Functions
\************************************************************************/
namespace StrTool {
size_t readBool(const char* buffer, bool* result, const char** errorStr/*=nullptr*/, bool arrayMode/*=false*/) {
  size_t n = 0; 
  if ((n = tryRead(FST("0"), buffer))) { *result = false; return n; }
  if ((n = tryRead(FST("false"), buffer))) { *result = false; return n; }
  if ((n = tryRead(FST("off"), buffer))) { *result = false; return n; }
  if ((n = tryRead(FST("1"), buffer))) { *result = true; return n; }
  if ((n = tryRead(FST("true"), buffer))) { *result = true; return n; }
  if ((n = tryRead(FST("on"), buffer))) { *result = true; return n; }
  if (errorStr) { *errorStr = FST("invalid boolean"); }
  return 0;
}

size_t readInteger(const char* buffer, int* result, const char** errorStr/*=nullptr*/, bool arrayMode/*=false*/) {
  size_t n = 0;
  int tmp = 0;
  bool is_negative = false;
  if (buffer[n] == '-') {
    is_negative = true;
    n++;
  }
  if (buffer[n] == '0' && buffer[n+1] == 'b') { return readBinary(buffer+n+2, result, is_negative, errorStr, arrayMode); }
  if (buffer[n] == '0' && buffer[n+1] == 'x') { return readHex(buffer+n+2, result, is_negative, errorStr, arrayMode); }

  while(!isWhiteSpaceOrEnd(buffer[n])) {
    char c = buffer[n]; 
    if (arrayMode && (c == ',' || c == ']')) break;
    if (c < '0' || c > '9') { 
      if (errorStr) { *errorStr = FST("invalid integer"); }
      return 0; 
    }
    tmp = tmp * 10 + (c - '0');
    n++;
  }
  if (is_negative) { tmp = -tmp; }
  while (isWhiteSpace(buffer[n])) { n++; } 
  *result = tmp;
  return n;
}

size_t readFloat(const char* buffer, float* result, const char** errorStr/*=nullptr*/, bool arrayMode/*=false*/) {
  size_t n = 0;
  float tmp = 0;
  float dec = 0.0;
  bool is_negative = false;
  if (buffer[n] == '-') {
    is_negative = true;
    n++;
  }

  while(!isWhiteSpaceOrEnd(buffer[n])) {
    char c = buffer[n]; 
    if (arrayMode && (c == ',' || c == ']')) break;
    if (c == '.') { 
      if (dec) {
        if (errorStr) { *errorStr = FST("invalid float"); }
        return 0;  
      }
      dec = 0.1;
    } else if (c < '0' || c > '9') { 
      if (errorStr) { *errorStr = FST("invalid float"); }
      return 0; 
    } else {
      if (dec == 0) { tmp = tmp * 10.0 + (c - '0'); }
      else { 
        tmp += (float)(c - '0') * dec; 
        dec /= 10.0; 
      }
    }  
    n++;
  }
  if (is_negative) { tmp = -tmp; }
  while (isWhiteSpace(buffer[n])) { n++; } 
  *result = tmp;
  return n;
}

size_t readBinary(const char* buffer, int* result, bool is_negative, const char** errorStr/*=nullptr*/, bool arrayMode/*=false*/) {
  size_t n = 0;
  int tmp = 0;
  if (!is_negative && buffer[n] == '-') {
    is_negative = true;
    n++;
  }
  while(!isWhiteSpaceOrEnd(buffer[n])) {
    char c = buffer[n]; 
    if (arrayMode && (c == ',' || c == ']')) break;
    if (c < '0' || c > '1') { 
      if (errorStr) { *errorStr = FST("invalid binary"); }
      return 0; 
    }
    tmp = (tmp << 1) + (c - '0');
    n++;
  }
  if (is_negative) { tmp = -tmp; }
  while (isWhiteSpace(buffer[n])) { n++; } 
  *result = tmp;
  return n;
}

size_t readHex(const char* buffer, int* result, bool is_negative, const char** errorStr/*=nullptr*/, bool arrayMode/*=false*/) {
  size_t n = 0;
  int tmp = 0;
  if (!is_negative && buffer[n] == '-') {
    is_negative = true;
    n++;
  }
  while(!isWhiteSpaceOrEnd(buffer[n])) {
    char c = toupper(buffer[n]);
    if (arrayMode && (c == ',' || c == ']')) break;
    if (c >= '0' && c <= '9') { 
      tmp = (tmp << 4) + (c - '0');    
    } else if (c >= 'A' && c <= 'F') {
      tmp = (tmp << 4) + (c - 'A' + 10);    
    } else {
      if (errorStr) { *errorStr = FST("invalid hex"); }
      return 0; 
    }
    n++;
  }
  if (is_negative) { tmp = -tmp; }
  while (isWhiteSpace(buffer[n])) { n++; } 
  *result = tmp;
  return n;
}

size_t readWord(const char* buffer, char* result, size_t size) {
  size_t n = 0;
  size_t i = 0;
  while (isWhiteSpace(buffer[n])) { n++; }
  while(!isWhiteSpaceOrEnd(buffer[n])) {
    if (i < (size-1)) { result[i++] = buffer[n]; }
    n++;
  }
  result[i] = '\0';
  while (isWhiteSpace(buffer[n])) { n++; } 
  return n;
}

size_t readIpAddr(const char* buffer, uint8_t ip[4], const char** errorStr/*=nullptr*/) {
  size_t n = 0;
  size_t i = 0;
  while(buffer[n] && i<4) {
    int tmp = atoi(&buffer[n]);
    if (tmp > 255 || tmp < -1) {
      if (errorStr) { *errorStr = FST("invalid ip address"); }
      return 0;
    }
    ip[i++] = tmp;
    while(buffer[n] == ' ' || (buffer[n] >= '0' && buffer[n] <= '9')) { n++; }
    if (buffer[n] == 0) { break; }
    if (buffer[n] == '.') { n++; }
    else {
      if (errorStr) { *errorStr = FST("invalid ip address"); }
      return 0;
    }
  }
  if (i != 4) {
    if (errorStr) { *errorStr = FST("too short ip address"); }
    return 0;
  }
  return n;
}

size_t tryRead(const char* str, const char* buffer) {
  size_t n = 0;
  while (true) {
    if (str[n] == '\0') {
      if (!isWhiteSpaceOrEnd(buffer[n])) {
        return 0;
      }
      while (isWhiteSpace(buffer[n])) { n++; } 
      return n;
    }
    if (tolower(buffer[n]) != tolower(str[n])) { return 0; }    
    n++;
  }
}

size_t toVarName(char* buffer, size_t size, const char* name) {
  size_t n = 0;
  while (n < size-1 && *name) {
    char c = *name++;
    if (c == ' ') { c = '_'; }
    buffer[n] = c;    
    n++;
  }
  buffer[n] = '\0';
  return n;
}

size_t toJsonName(char* buffer, size_t size, const char* name) {
  size_t n = 0;
  if (n < size-1) { buffer[n++] = '"'; }
  n += toVarName(buffer+n, size-n, name);
  if (n < size-1) { buffer[n++] = '"'; }
  if (n < size-1) { buffer[n++] = ':'; }
  buffer[n] = '\0';
  return n;
}

bool matchesVarName(const char* jName, const char* name) {
  while (*jName && *name) {
    if (*jName != *name) {  
      if (*jName != '_' || *name != ' ') { return false; }
    }
    jName++; 
    name++;
  }
  if (*name != '\0' || *jName != '\0') { return false; }
  return true;
}


// Checks if the beginning of the pName matches name.
size_t matchesNamePart(const char* pName, const char* name, bool matchCase/*=true*/) {
  size_t n = 0;
  while (*pName && *name) {
    char pc = *pName;
    char c = *name;
    if (!matchCase) { pc = tolower(pc); c = tolower(c); }
    if (pc != c) {  
      if (pc != '_' || c != ' ') { return 0; }
    }
    pName++; 
    name++;
    n++;
  }
  if (*name != '\0' || (*pName != '\0' && *pName != ' ' && *pName != '.')) { return 0; }
  return n;
}

uint32_t calculateCrc(const char* data, size_t size) {
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

};



bool parseIpAddr(uint8_t ip[4], const char* str) {
  int i = 0;
  while(*str && i<4) {
    ip[i++] = atoi(str);
    while(*str == ' ' || (*str >= '0' && *str <= '9')) { str++; }
    if (*str == 0) { break; }
    if (*str == '.') { str++; }
    else {
      DEBUG_print(F("Bad IP address character: ")); 
      DEBUG_println(*str); 
      return true;
    }
  }
  return false;
}
