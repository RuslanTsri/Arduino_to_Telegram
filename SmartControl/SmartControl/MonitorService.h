#ifndef MONITOR_SERVICE_H
#define MONITOR_SERVICE_H

#include <Arduino.h>
#include "SensorService.h"
#include "StorageService.h"

class MonitorService {
  public:
    // ВАЖЛИВО: У конструкторі більше немає TgBotService
    MonitorService(SensorService& sensorSvc, StorageService& storageSvc);
    
    void begin();
    void check(); 
    String getFullStatus(); 
    String getJsonStatus(); // НОВИЙ МЕТОД ДЛЯ ВЕБ-СЕРВЕРА
    
    void setMax(uint8_t id, float value);
    void setMin(uint8_t id, float value);
    void setName(uint8_t id, String newName);

  private:
    SensorService& _sensorSvc;
    StorageService& _storage; // Змінної _tgSvc більше немає!

    String _names[1]; 
    float _min[1];
    float _max[1];
    uint8_t _alarmState[1]; 
};

#endif