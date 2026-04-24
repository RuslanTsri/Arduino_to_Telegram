#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

#include <Arduino.h>

class SensorService {
  public:
    bool begin();
    float getTemperature(uint8_t channel);
    float getHumidity(uint8_t channel);
    
  private:
    void selectChannel(uint8_t channel);
};

#endif