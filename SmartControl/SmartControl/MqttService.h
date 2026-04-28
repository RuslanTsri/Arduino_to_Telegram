#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

#include <WiFi.h>
#include <PubSubClient.h>
#include "MonitorService.h"

class MqttService {
  public:
    MqttService(MonitorService& monitor);
    void begin();
    void maintain();
    void publishStatus(const String& payload);
    void publishAlarm(const String& message);
    void publishCommand(const String& topic, const String& cmd);

  private:
    WiFiClient _espClient;
    PubSubClient _client;
    MonitorService& _monitor;
    unsigned long _lastReconnectAttempt;
    
    void reconnect();
    static void callback(char* topic, byte* payload, unsigned int length);
};


#endif