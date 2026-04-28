#include "MqttService.h"
#include "Config.h"
#include "TelegramService.h"
#include <ArduinoJson.h> // Потрібно для красивого форматування статусу в Телеграм

// Глобальний вказівник потрібен для callback-функції PubSubClient
MqttService* globalMqttInstance = nullptr;

MqttService::MqttService(MonitorService& monitor) 
  : _client(_espClient), _monitor(monitor), _lastReconnectAttempt(0) {
  globalMqttInstance = this;
}

void MqttService::begin() {
  _client.setServer(MQTT_BROKER, MQTT_PORT);
  _client.setCallback(MqttService::callback);
  
  // Збільшуємо буфер, щоб вмістився довгий JSON статусу
  _client.setBufferSize(512); 
}

void MqttService::publishStatus(const String& payload) {
  if (_client.connected()) {
    _client.publish(MQTT_TOPIC_STATUS, payload.c_str());
  }
}

void MqttService::publishAlarm(const String& message) {
  if (_client.connected()) {
    _client.publish(MQTT_TOPIC_ALARM, message.c_str());
  }
}

// НОВА ФУНКЦІЯ: Дозволяє Телеграм-боту відправляти команди будь-якій зоні
void MqttService::publishCommand(const String& topic, const String& cmd) {
  if (_client.connected()) {
    _client.publish(topic.c_str(), cmd.c_str());
  }
}

void MqttService::maintain() {
  if (!_client.connected()) {
    long now = millis();
    if (now - _lastReconnectAttempt > 5000) {
      _lastReconnectAttempt = now;
      reconnect();
    }
  } else {
    _client.loop(); // Тримаємо з'єднання живим
  }
}

void MqttService::reconnect() {
  Serial.print("[MQTT] Підключення до хмари... ");
  if (_client.connect(MQTT_CLIENT_ID)) {
    Serial.println("УСПІШНО!");
    
    // 1. Підписуємось на команди САМЕ ДЛЯ ЦІЄЇ ПЛАТИ
    _client.subscribe(MQTT_TOPIC_CMD);
    Serial.printf("[MQTT] Підписано на команди: %s\n", MQTT_TOPIC_CMD);

    // 2. Підписуємось на статуси та алярми ВСІХ ПЛАТ (для Телеграм-бота)
    // Символ '+' означає будь-яку зону (1, 2, 3...)
    _client.subscribe("ipz234_tsri/zone/+/status");
    _client.subscribe("ipz234_tsri/zone/+/alarm");

  } else {
    Serial.print("ПОМИЛКА, код: ");
    Serial.println(_client.state());
  }
}

// Функція обробки команд, які надсилає Python-бот або наш вбудований Telegram-бот
void MqttService::callback(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  String cmd = "";
  for (unsigned int i = 0; i < length; i++) {
    cmd += (char)payload[i];
  }
  cmd.trim();
  
  if (!globalMqttInstance) return;

  // =========================================================
  // ЧАСТИНА 1: ПЕРЕХОПЛЕННЯ ДЛЯ ТЕЛЕГРАМ-БОТА (Master Node)
  // =========================================================
  
  // Якщо прийшов статус від якоїсь зони (наш або чужий)
  if (topicStr.endsWith("/status")) {
    // Парсимо JSON, щоб зробити красиво, як було в Python
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, cmd);
    
    if (!error) {
      String name = doc["name"].as<String>();
      String temp = doc["temp"].as<String>();
      if (temp == "null" || temp == "error") temp = "⚠️ Помилка датчика";
      String minT = doc["min"].as<String>();
      String maxT = doc["max"].as<String>();
      
      String formattedMsg = "✅ " + name + "\n🌡 Темп: " + temp + "°C\n⚙️ Ліміти: [" + minT + " - " + maxT + "]";
      telegram.sendTextMessage(formattedMsg);
    } else {
      // Якщо це не JSON (наприклад, звичайний текст), шлемо як є
      telegram.sendTextMessage(cmd);
    }
    return; // Завершуємо, бо це не команда
  }
  
  // Якщо прийшов алярм від якоїсь зони
  else if (topicStr.endsWith("/alarm")) {
    telegram.sendTextMessage("🚨 АЛЯРМ!\n" + cmd);
    return; // Завершуємо
  }


  // =========================================================
  // ЧАСТИНА 2: ВИКОНАННЯ КОМАНД САМЕ ДЛЯ ЦІЄЇ ПЛАТИ
  // =========================================================
  if (topicStr == String(MQTT_TOPIC_CMD)) {
    Serial.printf("\n[MQTT] Отримано команду: %s\n", cmd.c_str());

    if (cmd == "status") {
      String currentStatus = globalMqttInstance->_monitor.getFullStatus();
      globalMqttInstance->publishStatus(currentStatus);
    } 
    else if (cmd.startsWith("setmax ")) {
      float val = cmd.substring(7).toFloat();
      globalMqttInstance->_monitor.setMax(0, val);
      globalMqttInstance->publishStatus("✅ ПОРІГ MAX ОНОВЛЕНО: " + String(val));
      Serial.printf("✅ ПОРІГ MAX ОНОВЛЕНО: %.1f\n", val);
    }
    else if (cmd.startsWith("setmin ")) {
      float val = cmd.substring(7).toFloat();
      globalMqttInstance->_monitor.setMin(0, val);
      globalMqttInstance->publishStatus("✅ ПОРІГ MIN ОНОВЛЕНО: " + String(val));
      Serial.printf("✅ ПОРІГ MIN ОНОВЛЕНО: %.1f\n", val);
    }
    else if (cmd.startsWith("setname ")) {
      String newName = cmd.substring(8);
      globalMqttInstance->_monitor.setName(0, newName);
      globalMqttInstance->publishStatus("✅ НАЗВУ ЗМІНЕНО: " + newName);
      Serial.printf("✅ НАЗВУ ЗМІНЕНО: %s\n", newName.c_str());
    }
  }
}