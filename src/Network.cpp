#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <sys/time.h>

#include "Config.h"
#include "ConfigReg.h"
#include "Secret.h"
#include "Network.h"
#include "TcpServer.h"
#include "WebServer.h"


ConfigGroup configGroupNetwork(FST("WIFI"));

ConfigStr configSSID(FST("SSID"), 36, WIFI_SSID, 0, FST("Name of WIFI network"), &configGroupNetwork);
ConfigStr configPassword(FST("Password"), 64, WIFI_PASSWORD, 0, FST("Password for WIFI network"), &configGroupNetwork);
ConfigStr configHostname(FST("Hostname"), 32, HOSTNAME, 0, FST("Name of this deveice on the network"), &configGroupNetwork);

ConfigIpAddr configIpAddr(FST("IP"), 0, 0, FST("Fixed IP address of this device"), &configGroupNetwork);
ConfigIpAddr configGateway(FST("Gateway"), 0, 0, FST("Gateway IP address"), &configGroupNetwork);
ConfigIpAddr configSubnet(FST("Subnet"), 0, 0, FST("Subnet mask"), &configGroupNetwork);
ConfigIpAddr configDNS(FST("DNS"), 0, 0, FST("Domain Name Server"), &configGroupNetwork);

ConfigBool configNetworkDisabled(FST("disabled"), 0, 0, FST("Disable networking"), &configGroupNetwork);


int setenv(const char *, const char *, int);
void tzset();

void web_server_init();

TcpServer* tcpServer;
WebServer* webServer;

bool gotNtp = false;

void otaInit(const char* full_hostname);
void networkRun();
void wsRun();

void otaTask_(void* parameter ) {
    while (true) {
        networkRun();
        // vTaskDelay(500);
    }
}

void networkInit() {
  uint8_t* ip = configIpAddr.get();
    if (ip[0] || ip[1] || ip[2] || ip[3]) {
        // Static IP details...
        IPAddress ip(ip);
        IPAddress gateway(configGateway.get());
        IPAddress subnet(configSubnet.get());
        IPAddress dns(configDNS.get());
        WiFi.config(ip, dns, gateway, subnet);
    }

    // Configure the hostname
    char full_hostname[64];
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(full_hostname, sizeof(full_hostname)-1, "%s-%02X%02X", HOSTNAME, mac[4], mac[5]);
    WiFi.setHostname(full_hostname);

    DEBUG_printf(FST("\nConnecting '%s' to AP "), full_hostname);
    DEBUG_printf(FST("WIFI: %s  PW: %s\n"), configSSID.get(), configPassword.get());
    WiFi.mode(WIFI_STA);
    WiFi.begin(configSSID.get(), configPassword.get());   //WiFi connection
    int n = 80;
    while (WiFi.status() != WL_CONNECTED)
    {
        if(!n--) {
            DEBUG_println(FST("\nConnection Failed! Rebooting..."));
            delay(5000);
            ESP.restart();            
        }
        DEBUG_print(FST("."));
        delay(250);
    }

    DEBUG_println();
    DEBUG_print(FST("WiFi connected. IP address: "));
    DEBUG_println(WiFi.localIP());
    DEBUG_println();

#if ENABLE_OTA
    otaInit(full_hostname);
#endif

#if ENABLE_TELNET
    tcpServer = new TcpServer();
    tcpServer->begin();
    DEBUG_println(FST("Started Telnet"));
#endif

#if ENABLE_WEB_SERVER
    webServer = new WebServer();
    webServer->begin();
    DEBUG_println(FST("Started Web Server"));
#endif

#if ENABLE_NTP
  configTime(0, 0, NTP_SERVER_1, NTP_SERVER_2);
  //setenv("TZ", config.time_zone, 1);
  setenv("TZ", "PST8PDT,M3.2.0,M11.1.0", 1);
  tzset();
#endif

#ifdef USE_NETWORK_TASK
    xTaskCreate(
        networkTask_,   // Task function
        "net",          // String with name of task
        2048,           // Stack size in bytes
        NULL,           // Parameter passed as input of the task
        1,              // Priority of the task.
        NULL);          // Task handle.
#endif
}

void networkRun() {
  #if ENABLE_OTA
  ArduinoOTA.handle();
  #endif

  #if ENABLE_TELNET
  tcpServer->run();
  #endif

  #if ENABLE_WEB_SERVER
  webServer->run();
  #endif

}

IPAddress getLocalIp() { return WiFi.localIP(); }
String getWifiMac() { return WiFi.macAddress(); }
int getRSSI() { return WiFi.RSSI(); }


#if ENABLE_NTP
void getNtpTime() {
  uint32_t start = millis();
  gotNtp = false;
  while(getEpochTime() < 50000) {
    if ((millis()-start) > 6000) {
      DEBUG_println(F("\nFailed to get NTP time."));
      break;
    }
    delay(1);
  }
  gotNtp = true;
  char iso[32];
  getIsoTime(iso);
  DEBUG_print(F("NTP: "));
  DEBUG_print(gotNtp);
  DEBUG_print(F(" Time: "));
  DEBUG_print(iso);
  DEBUG_print(F(" Epoch: "));
  DEBUG_print(getEpochTime());
  DEBUG_print(F(" | NTP duration: "));
  DEBUG_println((millis() - start) * 0.001);  
  char bu[64];
  getTimeStr(bu, FST("%d.%m.%y %H:%M"));
  DEBUG_println(bu);
}
#endif

size_t getWifiId(char* buffer, size_t bSize) {
  size_t n = 0;
  const char* o = WiFi.macAddress().c_str();
  while (*o && n < bSize-1) {
    if (*o == ':') { o++; continue; }
    buffer[n++] = *o++;
  }
  buffer[n] = 0;
  return n;
}

void setupWiFiAccessPoint() {
  const char WiFiAPPSK[] PROGMEM = WIFI_ACCESS_POINT_PASSWORD; 
  WiFi.mode(WIFI_MODE_AP);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
  char apName[64];
  uint8_t mac[6];
  WiFi.softAPmacAddress(mac);
  sprintf_P(apName, FST("%s-%02X:%02X"), HOSTNAME, mac[4], mac[5]);
  DEBUG_print(F("\n\nPlease connect to Access Point: "));
  DEBUG_print(apName);
  DEBUG_printf(FST("\nPassword: %s\nhttp://192.168.4.1\n"), WIFI_ACCESS_POINT_PASSWORD);
  WiFi.softAP(apName, WiFiAPPSK);
  WiFi.setHostname(HOSTNAME);
}

void WifiDisconnect() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}


time_t getEpochTime() {
  time_t now = 0;
  time(&now);
  return now;
}

uint32_t getMillisDelay(uint32_t freq) {
  timeval tv;
  timezone tz;
  gettimeofday(&tv, &tz);
  struct tm * tm = localtime(&tv.tv_sec);
  uint32_t millis = (tm->tm_hour * 60*60*1000) + (tm->tm_min * 60*1000) + (tm->tm_sec * 1000) + (tv.tv_usec/1000); 
  return freq - (millis % freq) + 1;
}

size_t getIsoTime(char* buffer) {
  // 2018-05-27T21:33:19Z
  time_t now = 0;
  time(&now);
  return strftime(buffer, 32, FST("%FT%TZ"), gmtime(&now));
}

size_t getTimeStr(char* buffer, const char* fmt) {
  time_t now = 0;
  time(&now);
  return strftime(buffer, 32, fmt, localtime(&now));
}