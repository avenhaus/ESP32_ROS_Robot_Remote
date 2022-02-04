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
#include "Helper.h"
#include "Secret.h"
#include "Command.h"

ConfigGroup* ConfigGroup::mainGroup = nullptr;
size_t ConfigVar::count_ = 0;

/************************************************************************\
|* Config Commands
\************************************************************************/

CommandRegistry cmdRegConfig(FST("config"));

extern char fullHostname[];
Command cmdBasicConfig(FST("basics"), 
[] (const char* args, Print* stream) {
    #if ENABLE_SPIFFS
      #define SPIFFS_LOC "/sd"
    #else
      #define SPIFFS_LOC "No SD"
    #endif // ENABLE_SPIFFS
    stream->printf(FST("FW name: %s # FW version: %s # primary sd:" SPIFFS_LOC " # secondary sd:No SD # authentication:no # webcommunication: Sync: /ws # hostname:%s # tabs: " WEBUI_TABS " # start: " WEBUI_START_TAB "\n"),
    PROJECT_NAME, VERSION_NUMBER, fullHostname);
    return EC_OK;
},
FST("Get basic configuration"), &cmdRegConfig
);

Command cmdSaveConfig(FST("save"), 
[] (const char* args, Print* stream) {
    saveConfig();
    return EC_OK;
},
FST("Save configuration to EEPROM"), &cmdRegConfig
);

Command cmdLoadConfig(FST("load"), 
[] (const char* args, Print* stream) {
    loadConfig();
    return EC_OK;
},
FST("Load configuration from EEPROM"), &cmdRegConfig
);

Command cmdDefaultConfig(FST("def"), 
[] (const char* args, Print* stream) {
    defaultConfig();
    return EC_OK;
},
FST("Set default configuration"), &cmdRegConfig
);

Command cmdGetConfig(FST("get"), 
[] (const char* args, Print* stream) {
    ConfigGroup::mainGroup->toJson(stream, true);
    stream->println();
    return EC_OK;
},
FST("Get configuration as JSON"), &cmdRegConfig,
nullptr, CT_APP_JSON
);

Command cmdConfigVar(FST("var"), 
[] (const char* args, Print* stream) {
    ConfigVar* var = ConfigGroup::mainGroup->findVarByFullName(args, false);
    if (var) {
        while (*args && *args != ' ') { args++; };
        while (*args == ' ') { args++; }
        if (*args) {
            const char* errorStr = nullptr;
            var->setFromStr(args, &errorStr);
            if (stream && errorStr) { stream->print(errorStr); }
            return errorStr ? EC_BAD_REQUEST : EC_OK;
        } else {
            if (stream) { var->print(*stream); }
        }
    } else {
         if (stream) { stream->print(FST("unknown variable")); }
         return EC_NOT_FOUND;
    }
    if (stream) { stream->println(); }
    return EC_OK;
},
FST("Get or set configuration variable"), &cmdRegConfig, FST("<name> [value]"), CT_APP_JSON
);

Command cmdGetConfigUi(FST("ui"), 
[] (const char* args, Print* stream) {
    ConfigGroup::mainGroup->getWebUi(stream, true);
    stream->println();
    return EC_OK;
},
FST("Get configuration UI as JSON"), &cmdRegConfig,
nullptr, CT_APP_JSON
);


/************************************************************************\
|* Global Functions
\************************************************************************/

void saveConfig() {
  char buffer[CONFIG_BUFFER_SIZE];
  size_t size = ConfigGroup::mainGroup->toJsonStr(buffer, sizeof(buffer), true, CVF_SHOW_PASSWORD, CVF_NOT_PERSISTED);
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

size_t ConfigGroup::toJsonStr(char* buffer, size_t size, bool noName, uint16_t flags/*=0*/, uint8_t flagsMask/*=0*/) {
    size_t n = 0;
    if (!noName) { n += StrTool::toJsonName(buffer+n, size-n, name_); }
    if (n < size-1) { buffer[n++] = '{'; }
    bool first = true;
    for(auto v: vars_) {
      if (!first && (n < size-1)) { buffer[n++] = ','; }
      first = false; 
      n += v->toJsonStr(buffer+n, size-n, false, flags, flagsMask);
    }
    for(auto g: children_) {
      if (!first && (n < size-1)) { buffer[n++] = ','; }
      first = false; 
      n += g->toJsonStr(buffer+n, size-n, false, flags, flagsMask);
    }
    if (n < size-1) { buffer[n++] = '}'; }
    buffer[n] = '\0';
    return n;
}

size_t ConfigGroup::toJson(Print* stream, bool noName, uint16_t flags/*=0*/, uint8_t flagsMask/*=0*/) {
    char buffer[128];
    size_t size = sizeof(buffer);
    size_t n = 0;
    if (!noName) { n += StrTool::toJsonName(buffer, size, name_); }
    if (n < size-1) { buffer[n++] = '{'; }
    stream->write(buffer, n);
    bool first = true;
    for(auto v: vars_) {
      if (!first) { stream->write(','); n++; }
      first = false; 
      size_t m = v->toJsonStr(buffer, size, false, flags, flagsMask);
      stream->write(buffer, m);
      n += m;
    }
    for(auto g: children_) {
      if (!first) { stream->write(','); n++; }
      first = false; 
      n += g->toJson(stream, false, flags, flagsMask);
    }
    stream->write('}'); n++;
    return n;
}

size_t ConfigGroup::getWebUi(Print* stream, bool noName, uint16_t flags/*=0*/, uint8_t flagsMask/*=0*/) {
    char buffer[128];
    size_t size = sizeof(buffer);
    size_t n = 0;
    if (!noName) { n += StrTool::toJsonName(buffer, size, name_); }
    stream->write(buffer, n);
    n += stream->print(FST("{\"_INFO_\":{"));
    n += stream->printf(FST("\"L\":\"%s\""), name_);
    if (info_) {n += stream->printf(FST(",\"H\":\"%s\""), info_); }
    if (flags_) { n += stream->printf(FST(",\"F\":%d"), flags_); }
    bool first = true;
    n += stream->print(FST("},\"_VARS_\":["));
    for(auto v: vars_) {
      if (!v->isHidden()) {
        if (!first) { stream->write(','); n++; }
        first = false; 
        size_t m = v->getWebUi(stream, flags, flagsMask);
        n += m;
      }
    }
    stream->write(']'); n++;
    for(auto g: children_) {
      if (!first) { stream->write(','); n++; }
      first = false; 
      n += g->getWebUi(stream, false, flags, flagsMask);
    }
    stream->write('}'); n++;
    return n;
}

ConfigGroup* ConfigGroup::findChild(const char* name) {
    for(auto g: children_) {
      if (StrTool::matchesCleanName(name, g->name())) { return g; }
    }
    return nullptr;
}

ConfigVar* ConfigGroup::findVar(const char* name) {
    for(auto v: vars_) {
      if (StrTool::matchesCleanName(name, v->name())) { return v; }
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
      return StrTool::toCleanName(buffer, size, vars_.at(index)->name()); 
    }
    index -= vars_.size();
    for(auto g: children_) {
      if (index < g->size()) { 
        size_t n = StrTool::toCleanName(buffer, size, g->name_);
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
