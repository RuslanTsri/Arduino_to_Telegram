#include "SensorService.h"
#include "Config.h"
#include <Wire.h>
#include "Adafruit_HTU21DF.h"

Adafruit_HTU21DF htu = Adafruit_HTU21DF();

bool SensorService::begin() {
  return Wire.begin(SENSOR_SDA_PIN, SENSOR_SCL_PIN) && htu.begin();
}

float SensorService::getTemperature(uint8_t channel) {
  return htu.readTemperature();
}

float SensorService::getHumidity(uint8_t channel) {
  return htu.readHumidity();
}