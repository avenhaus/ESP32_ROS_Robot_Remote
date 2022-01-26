#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>

void networkInit();
String getWifiMac();
size_t getWifiId(char* buffer, size_t bSize=1<<30);
size_t WifiGetJsonInfo(char* buffer);
IPAddress getLocalIp();
int getRSSI();
void setupWiFiAccessPoint();
void WifiDisconnect();
void WifiSleep();
void WifiWake();
void getNtpTime();
time_t getEpochTime();
uint32_t getMillisDelay(uint32_t frequency);
size_t getIsoTime(char* buffer);
size_t getTimeStr(char* buffer, const char* fmt);
ErrorCode listWifiNetworksJson(Print& out, bool filterEmpty=true);

void printMac(Print& s, uint8_t *mac);
void wifiInfo(Print& s);

extern ConfigStr configHostname;
extern char fullHostname[];

#endif // NETWORK_H