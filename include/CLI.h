/*

 */

#ifndef CLI_H
#define CLI_H

#include <Stream.h>

void ReadSerial();

class CommandLineInterpreter {

public:
  CommandLineInterpreter(Stream* stream=nullptr) : stream(stream) { }
  const char* execute(const char* cmd);
  inline const char* setError(const char* text) { error = text; return text; }
  
protected:
  const char* help();
  const char* restart();
  const char* controlDebug(const char* cmd);
  const char* save();
  const char* load();
  const char* defConfig();
  const char* config(const char* cmd);

  Stream* stream;
  char buffer[256];
  const char* error;
};


#endif // CLI_H
