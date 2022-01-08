/*
 */

#include "Arduino.h"

#include "config.h"
#include "CLI.h"

#include <string.h>


static const char OK_TEXT[] PROGMEM = "OK";

const CommandLineInterpreter::PCongif parameters[] PROGMEM = {
  {FST(""), FST(""), FST(""), CommandLineInterpreter::pt_bool, 0, nullptr},  // End of list
};
static const size_t MAX_PARAMETER = sizeof(parameters)/sizeof(CommandLineInterpreter::PConfig);

const char* help_text = FST("\
debug [0|1|on|off|true|false]          Enable or disable debug logging\r\n\
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
  if ((n = tryRead(FST("HELP"), cmd))) { return help(); }
  if ((n = tryRead(FST("?"), cmd))) { return help(); }
  if ((n = tryRead(FST("DEBUG"), cmd))) { return controlDebug(cmd+n); }
  if ((n = tryRead(FST("RESTART"), cmd))) { return restart(); }

  size_t pn = 0;
  while (pn < MAX_PARAMETER) {
    if ((n = tryRead(parameters[pn].name, cmd))) { return controlParameter(&parameters[pn], cmd+n); }
    pn++;
  }

  return setError(FST("invalid command"));
}

const char* CommandLineInterpreter::help() {
  if (stream) {
    stream->println();
    const PCongif* p = parameters;
    while (p->name[0]) {
      sprintf(buffer, FST("%s [value]"), p->name);
      stream->printf(FST("%-38s Get/Set %s\r\n"), buffer, p->info);
      p++;
    }
    stream->println();
    stream->println(help_text);
  }
  return OK_TEXT;  
}

const char* CommandLineInterpreter::controlParameter(const PCongif* p, const char* cmd) {
  switch ( p->type ) {
    case pt_bool: {
      if ( cmd[0] == '\0' ) {
        if (!p->variable) { return nullptr; }
        sprintf_P(buffer, p->fmt, *((bool*)p->variable));
        return buffer;
      } 
      bool value = 0;
      readBool(cmd, &value);
      if (error) { return error; }
      if (p->variable) { *((bool*)p->variable) = value; }
      if (p->callback) { return (p->callback)(&value); }
      break;
    }

    case pt_int: {
      if ( cmd[0] == '\0' ) {
        if (!p->variable) { return nullptr; }
        sprintf_P(buffer, p->fmt, *((int*)p->variable));
        return buffer;
      } 
      int value = 0;
      readInteger(cmd, &value);
      if (error) { return error; }
      if (p->variable) { *((int*)p->variable) = value; }
      if (p->callback) { return (p->callback)(&value); }
      break;
    }

    case pt_float: {
      if ( cmd[0] == '\0' ) {
        if (!p->variable) { return nullptr; }
        sprintf_P(buffer, p->fmt, *((float*)p->variable));
        return buffer;
      } 
      float value = 0.0;
      readFloat(cmd, &value);
      if (error) { return error; }
      if (p->variable) { *((float*)p->variable) = value; }
      if (p->callback) { return (p->callback)(&value); }
      break;
    }

    default:
      return setError(FST("unknown parameter type not supported"));
  }
  return OK_TEXT;
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
  readBool(cmd, &state);
  if (error) { return error; }
  debugStream = state ? stream : nullptr;
  return OK_TEXT;  
}

const char* CommandLineInterpreter::restart() {
  ESP.restart();
  return OK_TEXT;  
}

size_t CommandLineInterpreter::readBool(const char* cmd, bool* result) {
  size_t n = 0; 
  if ((n = tryRead(FST("0"), cmd))) { *result = false; return n; }
  if ((n = tryRead(FST("false"), cmd))) { *result = false; return n; }
  if ((n = tryRead(FST("off"), cmd))) { *result = false; return n; }
  if ((n = tryRead(FST("1"), cmd))) { *result = true; return n; }
  if ((n = tryRead(FST("true"), cmd))) { *result = true; return n; }
  if ((n = tryRead(FST("on"), cmd))) { *result = true; return n; }
  setError(FST("invalid boolean"));
  return 0;
}

size_t CommandLineInterpreter::readInteger(const char* cmd, int* result) {
  size_t n = 0;
  int tmp = 0;
  bool is_negative = false;
  if (cmd[n] == '-') {
    is_negative = true;
    n++;
  }
  if (cmd[n] == '0' && cmd[n+1] == 'b') { return readBinary(cmd+n+2, result, is_negative); }
  if (cmd[n] == '0' && cmd[n+1] == 'x') { return readHex(cmd+n+2, result, is_negative); }

  while(!isWhiteSpaceOrEnd(cmd[n])) {
    char c = cmd[n]; 
    if (c < '0' || c > '9') { 
      setError(FST("invalid integer"));
      return 0; 
    }
    tmp = tmp * 10 + (c - '0');
    n++;
  }
  if (is_negative) { tmp = -tmp; }
  while (isWhiteSpace(cmd[n])) { n++; } 
  *result = tmp;
  return n;
}

size_t CommandLineInterpreter::readFloat(const char* cmd, float* result) {
  size_t n = 0;
  float tmp = 0;
  float dec = 0.0;
  bool is_negative = false;
  if (cmd[n] == '-') {
    is_negative = true;
    n++;
  }

  while(!isWhiteSpaceOrEnd(cmd[n])) {
    char c = cmd[n]; 
    if (c == '.') { 
      if (dec) {
        setError(FST("invalid float"));
        return 0;  
      }
      dec = 0.1;
    } else if (c < '0' || c > '9') { 
      setError(FST("invalid float"));
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
  while (isWhiteSpace(cmd[n])) { n++; } 
  *result = tmp;
  return n;
}

size_t CommandLineInterpreter::readBinary(const char* cmd, int* result, bool is_negative) {
  size_t n = 0;
  int tmp = 0;
  if (!is_negative && cmd[n] == '-') {
    is_negative = true;
    n++;
  }
  while(!isWhiteSpaceOrEnd(cmd[n])) {
    char c = cmd[n]; 
    if (c < '0' || c > '1') { 
      setError(FST("invalid binary"));
      return 0; 
    }
    tmp = (tmp << 1) + (c - '0');
    n++;
  }
  if (is_negative) { tmp = -tmp; }
  while (isWhiteSpace(cmd[n])) { n++; } 
  *result = tmp;
  return n;
}

size_t CommandLineInterpreter::readHex(const char* cmd, int* result, bool is_negative) {
  size_t n = 0;
  int tmp = 0;
  if (!is_negative && cmd[n] == '-') {
    is_negative = true;
    n++;
  }
  while(!isWhiteSpaceOrEnd(cmd[n])) {
    char c = toupper(cmd[n]);
    if (c >= '0' && c <= '9') { 
      tmp = (tmp << 4) + (c - '0');    
    } else if (c >= 'A' && c <= 'F') {
      tmp = (tmp << 4) + (c - 'A' + 10);    
    } else {
      setError(FST("invalid hex"));
      return 0; 
    }
    n++;
  }
  if (is_negative) { tmp = -tmp; }
  while (isWhiteSpace(cmd[n])) { n++; } 
  *result = tmp;
  return n;
}

size_t CommandLineInterpreter::readWord(const char* cmd, char* result, size_t size) {
  size_t n = 0;
  size_t i = 0;
  while (isWhiteSpace(cmd[n])) { n++; }
  while(!isWhiteSpaceOrEnd(cmd[n])) {
    if (i < (size-1)) { result[i++] = cmd[n]; }
    n++;
  }
  result[i] = '\0';
  while (isWhiteSpace(cmd[n])) { n++; } 
  return n;
}

size_t CommandLineInterpreter::readIpAddr(const char* cmd, uint8_t ip[4]) {
  size_t n = 0;
  size_t i = 0;
  while(cmd[n] && i<4) {
    int tmp = atoi(&cmd[n]);
    if (tmp > 255 || tmp < -1) {
      setError(FST("invalid ip address"));
      return 0;
    }
    ip[i++] = tmp;
    while(cmd[n] == ' ' || (cmd[n] >= '0' && cmd[n] <= '9')) { n++; }
    if (cmd[n] == 0) { break; }
    if (cmd[n] == '.') { n++; }
    else {
      setError(FST("invalid ip address"));
      return 0;
    }
  }
  if (i != 4) {
    setError(FST("too short ip address"));
    return 0;
  }
  return n;
}

size_t CommandLineInterpreter::tryRead(const char* str, const char* cmd) {
  size_t n = 0;
  while (true) {
    if (str[n] == '\0') {
      if (!isWhiteSpaceOrEnd(cmd[n])) {
        return 0;
      }
      while (isWhiteSpace(cmd[n])) { n++; } 
      return n;
    }
    if (tolower(cmd[n]) != tolower(str[n])) { return 0; }    
    n++;
  }
}
