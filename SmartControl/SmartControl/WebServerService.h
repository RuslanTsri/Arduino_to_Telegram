#ifndef WEBSERVER_SERVICE_H
#define WEBSERVER_SERVICE_H

#include <Arduino.h>
#include <WebServer.h>
#include "MonitorService.h"

class WebServerService {
  public:
    WebServerService(MonitorService& monitorSvc);
    void begin();
    void handleClient();

  private:
    WebServer _server;
    MonitorService& _monitor;

    // Обробники маршрутів (роути)
    void handleStatus();
    void handleSetMax();
    void handleSetMin();
    void handleSetName();
};

#endif