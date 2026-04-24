#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#include <WiFi.h>

class WifiService {
  public:
    WifiService(const char* ssid, const char* user, const char* pass);
    
    void connect();
    bool isConnected();
    void maintain(); 

  private:
    const char* _ssid;
    const char* _user;
    const char* _pass;
    unsigned long _offlineStartTime; // Таймер для критичних збоїв
};

#endif