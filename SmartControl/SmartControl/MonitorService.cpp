#include "MonitorService.h"
#include "Config.h"
#include <ArduinoJson.h>

MonitorService::MonitorService(SensorService& sensorSvc, StorageService& storageSvc) 
  : _sensorSvc(sensorSvc), _storage(storageSvc) {}

void MonitorService::begin() {
  _storage.begin();
  _names[0] = _storage.loadName(0, DEVICE_LABEL);
  _min[0] = _storage.loadTempMin(0, DEFAULT_TEMP_MIN);
  _max[0] = _storage.loadTempMax(0, DEFAULT_TEMP_MAX);
  _alarmState[0] = 0;
}

void MonitorService::setMax(uint8_t id, float value) {
  _max[0] = value;
  _storage.saveTempMax(0, value);
}

void MonitorService::setMin(uint8_t id, float value) {
  _min[0] = value;
  _storage.saveTempMin(0, value);
}

void MonitorService::setName(uint8_t id, String newName) {
  _names[0] = newName;
  _storage.saveName(0, newName);
}

void MonitorService::check() {
  // Ми залишаємо функцію check порожньою (або для сирени в майбутньому), 
  // бо тепер тривоги оброблятиме Python!
}

String MonitorService::getFullStatus() {
  float t = _sensorSvc.getTemperature(0);
  String status = "📍 " + _names[0] + "\n";
  status += "🌡 Темп: " + (isnan(t) ? "Помилка" : String(t, 1) + "°C") + "\n";
  status += "⚙️ Ліміти: [" + String(_min[0], 1) + " - " + String(_max[0], 1) + "]";
  return status;
}

// НОВИЙ МЕТОД ДЛЯ PYTHON-БОТА
String MonitorService::getJsonStatus() {
  float t = _sensorSvc.getTemperature(0);
  
  StaticJsonDocument<256> doc;
  doc["id"] = DEVICE_ID;
  doc["name"] = _names[0];
  
  if (isnan(t)) {
    doc["temp"] = "error";
  } else {
    doc["temp"] = round(t * 10.0) / 10.0; // Округлюємо до 1 знаку
  }
  
  doc["min"] = _min[0];
  doc["max"] = _max[0];

  String output;
  serializeJson(doc, output);
  return output;
}