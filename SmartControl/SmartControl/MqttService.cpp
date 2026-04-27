#include "MqttService.h"
#include "Config.h"

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
    // Щойно підключились - одразу підписуємось на топік команд
    _client.subscribe(MQTT_TOPIC_CMD);
    Serial.printf("[MQTT] Підписано на команди: %s\n", MQTT_TOPIC_CMD);
  } else {
    Serial.print("ПОМИЛКА, код: ");
    Serial.println(_client.state());
  }
}
void MqttService::publishAlarm(const String& message) {
  if (_client.connected()) {
    _client.publish(MQTT_TOPIC_ALARM, message.c_str());
  }
}

// Функція обробки команд, які надсилає Python-бот
void MqttService::callback(char* topic, byte* payload, unsigned int length) {
  String cmd = "";
  for (unsigned int i = 0; i < length; i++) {
    cmd += (char)payload[i];
  }
  cmd.trim();
  
  Serial.printf("\n[MQTT] Отримано команду з хмари: %s\n", cmd.c_str());

  if (!globalMqttInstance) return;

  // Обробка команд
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