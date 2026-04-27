#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#include <WiFi.h>

class WifiService {
  public:
    WifiService(const char* ssid, const char* user, const char* pass);
    
    void connect();
    bool isConnected();
    void maintain(); 
    void disconnect(); // Додано з шаблону для примусового відключення

  private:
    const char* _ssid;
    const char* _user;
    const char* _pass;
    unsigned long _offlineStartTime;
};

#endif