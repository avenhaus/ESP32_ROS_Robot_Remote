
#include <SPIFFS.h>

#include "Config.h"
#include "WebServer.h"


extern WebServer* webServer;

unsigned long next_update = 0;
//ControlStateData oldState;
bool stateSent = false;

void WebServer::run() {
  
    if (isReady() && ws.availableForWriteAll()) {
      char buffer[WS_BUFFER_SIZE];

      if (!stateSent) {
        //getControlJson(buffer, sizeof(buffer));
        ws.textAll(buffer);
        stateSent = true;
        DEBUG_println(FST("WS: Sent JSON config"));
      }

      if (millis() > next_update) {
        next_update = millis() + 200;
        //getNews(buffer, sizeof(buffer), &oldState);
        ws.textAll(buffer);
      }
    }



}

void WebServer::onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
               AwsEventType type, void* arg, uint8_t* data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    webServer->wsState = Connected;
    DEBUG_printf(FST("ws[%s][%u] connect\n"), server->url(), client->id());
    // client->printf("Hello Client %u :)", client->id());
    client->ping();
  } else if (type == WS_EVT_DISCONNECT) {
    webServer->wsState = NoConnection;
    stateSent = false;
    DEBUG_printf(FST("ws[%s][%u] disconnect\n"), server->url(), client->id());
  } else if (type == WS_EVT_ERROR) {
    DEBUG_printf(FST("ws[%s][%u] error(%u): %s\n"), server->url(), client->id(),
        *((uint16_t*)arg), (char*)data);
  } else if (type == WS_EVT_PONG) {
    DEBUG_printf(FST("ws[%s][%u] pong[%u]: %s\n"), server->url(), client->id(), len,
        (len) ? (char*)data : "");
      webServer->wsState = Ready;
  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo* info = (AwsFrameInfo*)arg;
    String msg = "";
    if (info->final && info->index == 0 && info->len == len) {
      // the whole message is in a single frame and we got all of it's data
      DEBUG_printf(FST("ws[%s][%u] %s-msg[%llu]\n"), server->url(), client->id(),
          (info->opcode == WS_TEXT) ? "txt" : "bin", info->len);

      if (info->opcode == WS_TEXT) {
        for (size_t i = 0; i < info->len; i++) {
          msg += (char)data[i];
        }
        DEBUG_printf(FST("%s\n\n"), msg.c_str());

        StaticJsonDocument<100> jsonDocRx;
        deserializeJson(jsonDocRx, msg);

        if (jsonDocRx.containsKey(F("led"))) {
            uint8_t ledState = jsonDocRx[F("led")];
            if (ledState == 1) {
            digitalWrite(LED_PIN, HIGH);
            }
            if (ledState == 0) {
            digitalWrite(LED_PIN, LOW);
            }
        }

        if (jsonDocRx.containsKey(F("cmd"))) {
            const char* cmd = jsonDocRx[F("cmd")];
            const char* result = webServer->cmdli.execute(cmd);
        }

        jsonDocRx.clear();
      }
    }
  }
}

void WebServer::begin() {
  /* Start web server and web socket server */
 // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false);
  });
  /*
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(SPIFFS, "/index.html.gz", "text/html");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  */
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon.ico", String(), false);
  });
  server.onNotFound(notFound);
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.begin();
}

