#include "StorageService.h"

void StorageService::begin() {
  // Відкриваємо простір імен "monitor" у режимі false (читання/запис)
  _prefs.begin("monitor", false);
}

void StorageService::saveTempMax(uint8_t id, float value) {
  String key = "tMax" + String(id);
  _prefs.putFloat(key.c_str(), value);
}

float StorageService::loadTempMax(uint8_t id, float defaultValue) {
  String key = "tMax" + String(id);
  return _prefs.getFloat(key.c_str(), defaultValue);
}

// Додамо також для MIN, бо це важливо для твоїх ванн
void StorageService::saveTempMin(uint8_t id, float value) {
  String key = "tMin" + String(id);
  _prefs.putFloat(key.c_str(), value);
}

float StorageService::loadTempMin(uint8_t id, float defaultValue) {
  String key = "tMin" + String(id);
  return _prefs.getFloat(key.c_str(), defaultValue);
}

void StorageService::saveName(uint8_t id, String name) {
  String key = "n" + String(id);
  _prefs.putString(key.c_str(), name);
}

String StorageService::loadName(uint8_t id, String defaultName) {
  String key = "n" + String(id);
  return _prefs.getString(key.c_str(), defaultName);
}