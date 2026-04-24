#ifndef CONFIG_H
#define CONFIG_H

// Підключаємо файл із нашими паролями та токенами
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

// --- ДЕФОЛТНІ ПОРОГИ (саме для цього пристрою) ---
#define DEFAULT_TEMP_MIN 50.0
#define DEFAULT_TEMP_MAX 85.0

#endif