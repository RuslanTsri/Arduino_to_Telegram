#ifndef STORAGE_SERVICE_H
#define STORAGE_SERVICE_H

#include <Arduino.h>
#include <Preferences.h>

class StorageService {
  public:
    void begin();
    void saveTempMax(uint8_t id, float value);
    float loadTempMax(uint8_t id, float defaultValue);
    
    // ОСЬ ЦІ ДВА РЯДКИ ТРЕБА ДОДАТИ:
    void saveTempMin(uint8_t id, float value);
    float loadTempMin(uint8_t id, float defaultValue);
    
    void saveName(uint8_t id, String name);
    String loadName(uint8_t id, String defaultName);
  
  private:
    Preferences _prefs;
};

#endif