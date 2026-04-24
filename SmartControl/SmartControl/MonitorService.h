#ifndef MONITOR_SERVICE_H
#define MONITOR_SERVICE_H

#include <Arduino.h>
#include "SensorService.h"
#include "TgBotService.h"
#include "StorageService.h"

class MonitorService {
  public:
    MonitorService(SensorService& sensorSvc, TgBotService& tgSvc, StorageService& storageSvc);
    
    void begin();
    void check(); 
    String getFullStatus(); 
    void setMax(uint8_t id, float value);
    void setMin(uint8_t id, float value);
    // ПЕРЕВІР, ЩО ЦЕЙ РЯДОК Є:
    void setName(uint8_t id, String newName);

  private:
    SensorService& _sensorSvc;
    TgBotService& _tgSvc;
    StorageService& _storage;

    String _names[1]; // Тепер у нас 1 датчик на плату
    float _min[1];
    float _max[1];
    uint8_t _alarmState[1]; 
};

#endif