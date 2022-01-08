#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <cstddef>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "Config.h"
#include "CLI.h"


#ifndef MAX_TCP_CONNECTIONS
#define MAX_TCP_CONNECTIONS 3
#endif

#ifndef WS_BUFFER_SIZE
#define WS_BUFFER_SIZE 512
#endif

#ifndef HTTP_PORT
#define HTTP_PORT 80
#endif

class WebServer {
public:
  enum WsState { NoConnection, Connected, Ready };
  WebServer(uint16_t port=HTTP_PORT,const char* wsPath = FST("/ws"))  : server(port), ws(wsPath), wsState(NoConnection), cmdli() { }
  void begin();
  void run();
  inline bool isConnected() { return wsState != NoConnection; }
  inline bool isReady() { return wsState == Ready; }

protected:
  static void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);
  static void notFound(AsyncWebServerRequest* request) {
    request->send(404, "text/plain", "Not found");
  }

  AsyncWebServer server;
  AsyncWebSocket ws;
  WsState wsState;
  CommandLineInterpreter cmdli;
};

#endif // WEB_SERVER_H
