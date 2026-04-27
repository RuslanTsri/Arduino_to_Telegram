#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include <WiFi.h>

class WifiController {
  public:
    int getSignalStrength();
    bool pingHost(const char* host, uint16_t port = 80);
    void connectToLast();
    void printDiagnostics();
};

#endif