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
  inline bool isWhiteSpace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
  inline bool isWhiteSpaceOrEnd(char c) { return c == '\0' || c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
  inline const char* setError(const char* text) { error = text; return text; }

  typedef enum PType {pt_bool, pt_int, pt_float, pt_str, pt_ip} PType;
  typedef struct PCongif {
    const char* name;
    const char* fmt;
    const char* info;
    PType type;
    void* variable;
    const char* (*callback)(void* variable);
  } PConfig;

  static const char* controlServo(void* vp);

protected:
  const char* help();
  const char* restart();
  const char* controlDebug(const char* cmd);
  const char* controlParameter(const PCongif* p, const char* cmd);

  size_t readBool(const char* cmd, bool* result);
  size_t readInteger(const char* cmd, int* result);
  size_t readFloat(const char* cmd, float* result);
  size_t readWord(const char* cmd, char* result, size_t size);
  size_t readBinary(const char* cmd, int* result, bool is_negative=false);
  size_t readHex(const char* cmd, int* result, bool is_negative=false);
  size_t readIpAddr(const char* cmd, uint8_t ip[4]);
  size_t tryRead(const char* str, const char* cmd);

  Stream* stream;
  char buffer[256];
  const char* error;
};


#endif // CLI_H
