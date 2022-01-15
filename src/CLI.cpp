/*
 */

#include "Arduino.h"

#include "Config.h"
#include "ConfigReg.h"
#include "CLI.h"

#include <string.h>

static const char OK_TEXT[] PROGMEM = "OK";

const char* help_text = FST("\
debug [0|1|on|off|true|false]          Enable or disable debug logging\r\n\
save                                   Save configuration to EEPROM\r\n\
load                                   Load configuration from EEPROM\r\n\
default                                Load default configuration\r\n\
config                                 Current configuration as JSON\r\n\
restart                                Restart the controller\r\n\
help                                   Print help\r\n\
?                                      Print help\r\n\
\r\n");

char serial_command[COMMAND_BUFFER_SIZE];
size_t serial_command_index = 0;
CommandLineInterpreter serial_cli = CommandLineInterpreter(&Serial);

void ReadSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    serial_command[serial_command_index++] = c;
    if (c == '\n' || c == '\r') { 
      serial_command[serial_command_index-1] = '\0';
      serial_command_index = 0;
      DEBUG_print(F("Execute: "));
      DEBUG_println(serial_command);
      DEBUG_println(serial_cli.execute(serial_command));
    }
    if (serial_command_index > COMMAND_BUFFER_SIZE-1) {
      serial_command_index = COMMAND_BUFFER_SIZE-1;
    }
  }
}

const char* CommandLineInterpreter::execute(const char* cmd) {
  error = nullptr;
  size_t n = 0; 
  if (*cmd == '#') { return FST(""); } // Comment line
  if ((n = StrTool::tryRead(FST("HELP"), cmd))) { return help(); }
  if ((n = StrTool::tryRead(FST("?"), cmd))) { return help(); }
  if ((n = StrTool::tryRead(FST("DEBUG"), cmd))) { return controlDebug(cmd+n); }
  if ((n = StrTool::tryRead(FST("RESTART"), cmd))) { return restart(); }
  if ((n = StrTool::tryRead(FST("SAVE"), cmd))) { return save(); }
  if ((n = StrTool::tryRead(FST("LOAD"), cmd))) { return load(); }
  if ((n = StrTool::tryRead(FST("DEFAULT"), cmd))) { return defConfig(); }
  if ((n = StrTool::tryRead(FST("CONFIG"), cmd))) { return config(cmd+n); }

  ConfigVar* var = ConfigGroup::mainGroup->findVarByFullName(cmd, false);
  if (var) {
    while (*cmd && *cmd != ' ') { cmd++; };
    while (*cmd == ' ') { cmd++; }
    if (*cmd) {
      const char* errorStr = nullptr;
      var->setFromStr(cmd, &errorStr);
      if (errorStr) { setError(errorStr); return errorStr; }
      return OK_TEXT;
    } else {
      var->toStr(buffer, sizeof(buffer));
      return buffer;
    }
  }

  return setError(FST("invalid command"));
}

const char* CommandLineInterpreter::help() {
  if (stream) {
    stream->println();
    //for(auto i : *ConfigGroup::mainGroup) {
    for (ConfigGroup::Iterator i = ConfigGroup::mainGroup->begin(), end = ConfigGroup::mainGroup->end(); i != end; i++) {
      ConfigVar& v = *i;
      if (v.hidden()) { continue; }
      char name[64];
      i.getVarName(name, sizeof(name));
      sprintf(buffer, FST("%s [%s]"), name, v.type_help());
      stream->printf(FST("%-38s Get/Set %s\r\n"), buffer, v.info() ? v.info() : FST(""));
    }

    stream->println();
    stream->println(help_text);
  }
  return OK_TEXT;  
}

const char* CommandLineInterpreter::save() {
  saveConfig();
  return OK_TEXT;  
}

const char* CommandLineInterpreter::load() {
  loadConfig();
  return OK_TEXT;  
}

const char* CommandLineInterpreter::defConfig() {
  defaultConfig();
  return OK_TEXT;  
}

const char* CommandLineInterpreter::config(const char* cmd) {
  size_t size = ConfigGroup::mainGroup->toJsonStr(buffer, sizeof(buffer), true);
  return buffer;
}

const char* CommandLineInterpreter::controlDebug(const char* cmd) {
  bool state = false;
  if ( cmd[0] == '\0' ) {
    if (debugStream == nullptr) { 
      return FST("off");
    } else if (debugStream == &Serial) {
      return FST("serial");
    } else if (debugStream == stream) {
      return FST("here");
    } else {
      return FST("on");
    }
  }
  const char* errorStr = nullptr;
  StrTool::readBool(cmd, &state, &errorStr);
  if (errorStr) { setError(errorStr); return error; }
  debugStream = state ? stream : nullptr;
  return OK_TEXT;  
}

const char* CommandLineInterpreter::restart() {
  ESP.restart();
  return OK_TEXT;  
}

