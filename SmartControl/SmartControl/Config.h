#ifndef CONFIG_H
#define CONFIG_H

// Підключаємо файл із нашими паролями
#include "Secrets.h" 

// --- УНІКАЛЬНИЙ ID ПРИСТРОЮ ---
#define DEVICE_ID 1                // Змінюй на 2, 3... для інших плат
#define DEVICE_LABEL "Ванна ГАРЯЧА" // Назва саме цього датчика

// --- ПІНИ ДЛЯ ОДНОГО ДАТЧИКА ---
#define SENSOR_SDA_PIN 21
#define SENSOR_SCL_PIN 22
#define STATUS_LED 2

// --- НАЛАШТУВАННЯ WIFI ТА TG (Беремо з Secrets.h) ---
#define WIFI_SSID SECRET_WIFI_SSID
#define WIFI_USER SECRET_WIFI_USER
#define WIFI_PASS SECRET_WIFI_PASS

#define BOT_TOKEN SECRET_BOT_TOKEN
#define CHAT_ID   SECRET_CHAT_ID

// --- ДЕФОЛТНІ ПОРОГИ ---
#define DEFAULT_TEMP_MIN 50.0
#define DEFAULT_TEMP_MAX 85.0
#define ALARM_THRESHOLD 2.0

// =======================================================
// --- НАЛАШТУВАННЯ ХМАРИ (MQTT) ---
// =======================================================
#define MQTT_BROKER "broker.emqx.io" // Безкоштовний стабільний сервер
#define MQTT_PORT 1883

// ⚠️ ДЛЯ КОЖНОЇ ПЛАТИ МАЄ БУТИ СВІЙ УНІКАЛЬНИЙ CLIENT ID (наприклад Vanna_Dev_2)
#define MQTT_CLIENT_ID "ESP32_DEV_1" 

// ⚠️ УНІКАЛЬНІ ШЛЯХИ ДЛЯ ПЛАТИ №1
// Змінюй /1/ на /2/ для другої плати
#define MQTT_TOPIC_STATUS SECRET_MQTT_TOPIC_STATUS // Сюди плата кидає дані
#define MQTT_TOPIC_CMD    SECRET_MQTT_TOPIC_CMD    // Звідси плата слухає команди від Python
#define MQTT_TOPIC_ALARM  SECRET_MQTT_TOPIC_ALARM 

#endif