#include "WifiService.h"
#include <Arduino.h>
#include "esp_eap_client.h" // Потрібно для Enterprise мереж (універ)

WifiService::WifiService(const char* ssid, const char* user, const char* pass) 
  : _ssid(ssid), _user(user), _pass(pass), _offlineStartTime(0) {}

void WifiService::connect() {
  Serial.print("\n=== WiFi Connection Start ===\n");
  Serial.print("Target SSID: '"); Serial.print(_ssid); Serial.println("'");

  // 1. Жорстке скидання
  WiFi.disconnect(true, true);
  delay(100);
  WiFi.mode(WIFI_STA);
  
  // ВМИКАЄМО АПАРАТНЕ АВТО-ПЕРЕПІДКЛЮЧЕННЯ
  WiFi.setAutoReconnect(true); 
  delay(100);

  // 2. Універсальна логіка підключення
  if (strlen(_user) > 0) {
    Serial.println("Mode: WPA2 Enterprise");
    esp_eap_client_set_identity((uint8_t *)_user, strlen(_user));
    esp_eap_client_set_username((uint8_t *)_user, strlen(_user));
    esp_eap_client_set_password((uint8_t *)_pass, strlen(_pass));
    esp_wifi_sta_enterprise_enable();
    WiFi.begin(_ssid);
  } else if (strlen(_pass) > 0) {
    Serial.println("Mode: WPA2 Personal");
    WiFi.begin(_ssid, _pass);
  } else {
    Serial.println("Mode: Open Network");
    WiFi.begin(_ssid);
  }

  // 3. Чекаємо першого підключення (15 секунд)
  int max_attempts = 30;
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < max_attempts) {
    delay(500); Serial.print(".");
    attempt++;
  }
  Serial.println("");

  // 4. Перевірка результату
  if (isConnected()) {
    Serial.println(">>> SUCCESS! <<<");
    Serial.print("IP: "); Serial.println(WiFi.localIP());
    _offlineStartTime = 0; // Скидаємо таймер ЧП
  } else {
    Serial.println(">>> FAILED! Will auto-retry in background. <<<");
  }
  Serial.println("=============================\n");
}

bool WifiService::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void WifiService::maintain() {
  // Якщо є мережа
  if (isConnected()) {
    if (_offlineStartTime != 0) {
        Serial.println("WiFi Restored Automatically!");
        _offlineStartTime = 0; // Скидаємо таймер
    }
    return; // Виходимо, бо все ок
  }

  // --- ЛОГІКА ДІЙ ПРИ ЧП (Немає мережі) ---

  // Щойно втратили інтернет - засікаємо час
  if (_offlineStartTime == 0) {
    _offlineStartTime = millis(); 
    Serial.println("WARNING: WiFi lost! Waiting for auto-reconnect...");
  }

  // Якщо мережі немає 3 хвилини (180 000 мс)
  if (millis() - _offlineStartTime > 180000) {
    Serial.println("CRITICAL ERROR: WiFi offline for 3 mins.");
    Serial.println("Rebooting ESP32 to prevent hardware freeze...");
    delay(1000);
    ESP.restart(); // Жорстке перезавантаження мікроконтролера
  }
}