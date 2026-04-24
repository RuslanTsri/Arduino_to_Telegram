#include "Config.h" 
#include "WifiService.h"
#include "WifiController.h"
#include "SensorService.h" 
#include "MonitorService.h"

#include "StorageService.h"

// 1. Ініціалізація сервісів
WifiService wifi(WIFI_SSID, WIFI_USER, WIFI_PASS);
WifiController wifiCtrl;
SensorService sensor; 
WebServerService webServer(monitor);
StorageService storage;
MonitorService monitor(sensor, tgBot, storage); 

// 2. Таймери та стан
bool isSensorReady = false;
unsigned long lastMonitorCheck = 0; 
unsigned long lastTgPoll = 0; 
const int POLL_INTERVAL = 5000;
unsigned long pollOffset = (DEVICE_ID - 1) * 800; 

void setup() {
  Serial.begin(115200);
  pinMode(STATUS_LED, OUTPUT);

  isSensorReady = sensor.begin();
  monitor.begin();
  wifi.connect();

  if (wifi.isConnected()) {
    delay(pollOffset);
    webServer.begin();
  }
}

void loop() {
  wifi.maintain();

  if (wifi.isConnected()) {
    webServer.handleClient(); 
  }

  // --- АВТОМАТИЧНИЙ МОНІТОРИНГ ЛІНІЇ ---
  if (isSensorReady && millis() - lastMonitorCheck > 5000) {
    monitor.check();
    lastMonitorCheck = millis();
  }

  // --- РОЗУМНЕ ОПИТУВАННЯ ТЕЛЕГРАМ ---
  if (wifi.isConnected() && (millis() % POLL_INTERVAL) > pollOffset && (millis() % POLL_INTERVAL) < (pollOffset + 600)) {
    if (millis() - lastTgPoll > 1000) {
      String incomingCmd = tgBot.getUpdates();
      if (incomingCmd != "") {
        handleTelegramCommand(incomingCmd);
      }
      lastTgPoll = millis();
    }
  }

  // --- ІНДИКАЦІЯ СТАТУСУ ---
  digitalWrite(STATUS_LED, wifi.isConnected() ? HIGH : (millis() / 500) % 2);

  // --- ОБРОБКА КОМАНД З КОНСОЛІ (SERIAL) ---
  if (Serial.available() > 0) {
    String serialCmd = Serial.readStringUntil('\n');
    serialCmd.trim();

    if (serialCmd == "status") {
      Serial.println(monitor.getFullStatus());
    } 
    else if (serialCmd == "temp") {
      Serial.printf("Поточна температура: %.1f °C\n", sensor.getTemperature(0));
    }
    else if (serialCmd == "diag") {
      wifiCtrl.printDiagnostics();
    }
    // Локальне налаштування через кабель
    else if (serialCmd.startsWith("setmax ")) {
      float val = serialCmd.substring(7).toFloat();
      monitor.setMax(0, val);
      Serial.printf("✅ ПОРІГ MAX ОНОВЛЕНО: %.1f (Збережено)\n", val);
    }
    else if (serialCmd.startsWith("setmin ")) {
      float val = serialCmd.substring(7).toFloat();
      monitor.setMin(0, val);
      Serial.printf("✅ ПОРІГ MIN ОНОВЛЕНО: %.1f (Збережено)\n", val);
    }
    else if (serialCmd.startsWith("setname ")) {
      String newName = serialCmd.substring(8);
      monitor.setName(0, newName);
      Serial.printf("✅ НАЗВУ ЗМІНЕНО: %s (Збережено)\n", newName.c_str());
    }
  }
}

// --- ЦЕНТРАЛЬНИЙ ОБРОБНИК ТЕЛЕГРАМ-КОМАНД ---
void handleTelegramCommand(String cmd) {
  String id = String(DEVICE_ID);

  // 1. ЗАГАЛЬНІ КОМАНДИ
  if (cmd == "/status") {
    delay((DEVICE_ID - 1) * 700);
    tgBot.sendMessage(monitor.getFullStatus());
  } 
  else if (cmd == "/diag") {
    delay((DEVICE_ID - 1) * 700);
    String d = "📶 Діагностика [" + id + "]\n";
    d += "Сигнал: " + String(wifiCtrl.getSignalStrength()) + " dBm\n";
    d += "IP: " + WiFi.localIP().toString();
    tgBot.sendMessage(d);
  }

  // 2. ПЕРСОНАЛЬНІ КОМАНДИ (наприклад: /setmax1 80)
  if (cmd.startsWith("/setmax" + id)) {
    float val = cmd.substring(8).toFloat(); // витягуємо число після /setmax1
    monitor.setMax(0, val);
    tgBot.sendMessage("✅ [" + id + "] Max поріг змінено на " + String(val));
  }
  else if (cmd.startsWith("/setmin" + id)) {
    float val = cmd.substring(8).toFloat();
    monitor.setMin(0, val);
    tgBot.sendMessage("✅ [" + id + "] Min поріг змінено на " + String(val));
  }
  else if (cmd.startsWith("/setname" + id)) {
    String newName = cmd.substring(9); // витягуємо текст після /setname1
    monitor.setName(0, newName);
    tgBot.sendMessage("✅ [" + id + "] Нова назва: " + newName);
  }
  else if (cmd == "/ping" + id) {
    bool ok = wifiCtrl.pingHost("8.8.8.8");
    tgBot.sendMessage("🌐 [" + id + "] Зв'язок: " + (ok ? "OK" : "FAILED"));
  }
}