#include "WebServerService.h"
#include <ArduinoJson.h>

// Сервер працює на стандартному веб-порту 80
WebServerService::WebServerService(MonitorService& monitorSvc) 
  : _server(80), _monitor(monitorSvc) {}

void WebServerService::begin() {
  // Налаштовуємо маршрути (що буде, якщо перейти за посиланням)
  _server.on("/status", HTTP_GET, std::bind(&WebServerService::handleStatus, this));
  _server.on("/setmax", HTTP_GET, std::bind(&WebServerService::handleSetMax, this));
  _server.on("/setmin", HTTP_GET, std::bind(&WebServerService::handleSetMin, this));
  _server.on("/setname", HTTP_GET, std::bind(&WebServerService::handleSetName, this));

  _server.begin();
  Serial.println("🌐 WebServer запущено на порту 80");
}

void WebServerService::handleClient() {
  _server.handleClient(); // Постійно слухаємо ефір
}

void WebServerService::handleStatus() {
  // Віддаємо Python-боту красивий JSON з усіма даними
  _server.send(200, "application/json", _monitor.getJsonStatus());
}

void WebServerService::handleSetMax() {
  if (_server.hasArg("val")) {
    float val = _server.arg("val").toFloat();
    _monitor.setMax(0, val);
    _server.send(200, "application/json", "{\"status\":\"ok\", \"new_max\":" + String(val) + "}");
  } else {
    _server.send(400, "application/json", "{\"error\":\"Missing 'val' parameter\"}");
  }
}

void WebServerService::handleSetMin() {
  if (_server.hasArg("val")) {
    float val = _server.arg("val").toFloat();
    _monitor.setMin(0, val);
    _server.send(200, "application/json", "{\"status\":\"ok\", \"new_min\":" + String(val) + "}");
  } else {
    _server.send(400, "application/json", "{\"error\":\"Missing 'val' parameter\"}");
  }
}

void WebServerService::handleSetName() {
  if (_server.hasArg("val")) {
    String newName = _server.arg("val");
    _monitor.setName(0, newName);
    _server.send(200, "application/json", "{\"status\":\"ok\", \"new_name\":\"" + newName + "\"}");
  } else {
    _server.send(400, "application/json", "{\"error\":\"Missing 'val' parameter\"}");
  }
}