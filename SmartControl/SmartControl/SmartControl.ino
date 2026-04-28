#include <WiFi.h>
#include "Config.h" 
#include "WifiService.h"
#include "WifiController.h"
#include "SensorService.h" 
#include "MonitorService.h"
#include "StorageService.h"
#include "MqttService.h" 
#include "TelegramService.h"

// 1. Ініціалізація сервісів
WifiService wifi(WIFI_SSID, WIFI_USER, WIFI_PASS);
WifiController wifiCtrl;
SensorService sensor; 
StorageService storage;
MonitorService monitor(sensor, storage); 
MqttService mqtt(monitor); 

// 2. Таймери та стан
bool isSensorReady = false;
unsigned long lastMeasureTime = 0; 
bool isAlarmActive = false; // Прапорець, щоб не спамити алярмами

void setup() {
  Serial.begin(115200);

Serial.println("\n[Час синхронізовано]");
  telegram.begin(true);
  Serial.setDebugOutput(true); 
  delay(1000); 

  Serial.println("\n\n====================================");
  Serial.println("🚀 СТАРТ ПРОГРАМИ (MQTT Enterprise Edition)!");
  Serial.println("====================================\n");

  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info){
      if(event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
          Serial.printf("\n🛑 [WIFI] Відключено! Причина: %d\n", info.wifi_sta_disconnected.reason);
      }
  });

  pinMode(STATUS_LED, OUTPUT);

  isSensorReady = sensor.begin();
  storage.begin(); 
  monitor.begin();
  mqtt.begin(); 
  wifi.connect();
}

void loop() {
  wifi.maintain();

  if (wifi.isConnected()) {
    mqtt.maintain(); 
  }
  telegram.loop();
  // --- ЛОГІКА МОНІТОРИНГУ: ВИМІРЮВАННЯ ЩОСЕКУНДИ ---
  if (isSensorReady && millis() - lastMeasureTime > 1000) {
    lastMeasureTime = millis();
    monitor.check(); 
    
    float currentTemp = sensor.getTemperature(0);
    
    // Використовуємо ТВОЇ назви функцій з StorageService
    float tMax = storage.loadTempMax(0, DEFAULT_TEMP_MAX);
    float tMin = storage.loadTempMin(0, DEFAULT_TEMP_MIN);
    String devName = storage.loadName(0, DEVICE_LABEL);

    // ПЕРЕВІРКА НА МЕЖІ + ALARM_THRESHOLD (який ми додали в Config.h)
    bool isTooHot = currentTemp > (tMax + ALARM_THRESHOLD);
    bool isTooCold = currentTemp < (tMin - ALARM_THRESHOLD);

    if (isTooHot || isTooCold) {
      if (!isAlarmActive) {
        String msg = "🚨 КРИТИЧНИЙ АЛЯРМ! [" + devName + "]\n";
        msg += isTooHot ? "🔥 ПЕРЕГРІВ: " : "❄️ ОХОЛОДЖЕННЯ: ";
        msg += String(currentTemp, 1) + "°C\n";
        msg += "(Ліміт: " + String(isTooHot ? tMax : tMin, 0) + "°C + поріг " + String(ALARM_THRESHOLD, 0) + "°C)";
        
        mqtt.publishAlarm(msg); 
        isAlarmActive = true; 
        Serial.println("[MQTT] Алярм відправлено!");
      }
    } 
    else if (currentTemp <= tMax && currentTemp >= tMin) {
      if (isAlarmActive) {
        String msg = "✅ СТАН СТАБІЛІЗОВАНО [" + devName + "]\nТемпература в нормі: " + String(currentTemp, 1) + "°C";
        mqtt.publishAlarm(msg);
        isAlarmActive = false;
        Serial.println("[MQTT] Об'єкт в нормі.");
      }
    }
  }
  // Світлодіод: горить стабільно якщо є MQTT, мигає якщо тільки WiFi
  digitalWrite(STATUS_LED, wifi.isConnected() ? HIGH : (millis() / 500) % 2);

  // --- ОБРОБКА КОМАНД З SERIAL (для локального дебагу) ---
 if (Serial.available() > 0) {
    String serialCmd = Serial.readStringUntil('\n');
    serialCmd.trim();

    if (serialCmd == "status") {
      Serial.println(monitor.getFullStatus());
    } 
    else if (serialCmd.startsWith("setmax ")) {
      float val = serialCmd.substring(7).toFloat();
      storage.saveTempMax(0, val); // Твій метод збереження
      Serial.printf("✅ MAX збережено: %.1f\n", val);
    }
    else if (serialCmd.startsWith("setmin ")) {
      float val = serialCmd.substring(7).toFloat();
      storage.saveTempMin(0, val); // Твій метод збереження
      Serial.printf("✅ MIN збережено: %.1f\n", val);
    }
    else if (serialCmd.startsWith("setname ")) {
      String newName = serialCmd.substring(8);
      storage.saveName(0, newName); // Твій метод збереження
      Serial.printf("✅ NAME збережено: %s\n", newName.c_str());
    }
  }
}