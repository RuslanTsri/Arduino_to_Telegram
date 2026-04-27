#include "WifiService.h"
#include <Arduino.h>
#include "esp_eap_client.h" 
#include <esp_wifi.h>

WifiService::WifiService(const char* ssid, const char* user, const char* pass) 
  : _ssid(ssid), _user(user), _pass(pass), _offlineStartTime(0) {}

void WifiService::connect() {
  Serial.println("\n[WiFi] Спроба прориву через Enterprise...");

  WiFi.disconnect(true, true);
  delay(200);
  WiFi.mode(WIFI_STA);

  // Маскування під iPhone залишаємо - це допомагає пройти фільтри заліза
  WiFi.setHostname("iPhone-Ruslan");
  uint8_t fakeMac[] = {0x48, 0x74, 0x6E, 0x11, 0x22, 0x33}; 
  esp_wifi_set_mac(WIFI_IF_STA, &fakeMac[0]);

  if (strlen(_user) > 0) {
    // Очищення старих даних авторизації
    esp_eap_client_set_identity(NULL, 0); 
    
    // Встановлюємо Identity та Username (для ZSTU часто мають бути однакові)
    esp_eap_client_set_identity((uint8_t *)_user, strlen(_user));
    esp_eap_client_set_username((uint8_t *)_user, strlen(_user));
    esp_eap_client_set_password((uint8_t *)_pass, strlen(_pass));

    // ХАК: Примусове відключення перевірки сертифіката сервера
    // Ми кажемо, що CA cert порожній, але метод фази 2 - MSCHAPV2
    esp_eap_client_set_ca_cert(NULL, 0);
    esp_eap_client_set_ttls_phase2_method(ESP_EAP_TTLS_PHASE2_MSCHAPV2);

    esp_wifi_sta_enterprise_enable();
    WiFi.begin(_ssid);
  } else {
    WiFi.begin(_ssid, _pass);
  }

  int numberOfTries = 50; 
  while (WiFi.status() != WL_CONNECTED && numberOfTries > 0) {
    delay(500);
    Serial.print(".");
    numberOfTries--;
  }

  if (isConnected()) {
    Serial.println("\n[WiFi] ПЕРЕМОГА! Підключено до ztu.edu.ua");
    Serial.print("[WiFi] IP: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[WiFi] Навіть хак не допоміг. Перевір логін/пароль.");
  }
}

void WifiService::disconnect() {
  Serial.println("[WiFi] Disconnecting from WiFi!");
  if (WiFi.disconnect(true, false)) {
    Serial.println("[WiFi] Disconnected successfully!");
  }
}
bool WifiService::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void WifiService::maintain() {
  if (isConnected()) {
    if (_offlineStartTime != 0) {
        Serial.println("[WiFi] Connection Restored Automatically!");
        _offlineStartTime = 0; 
    }
    return; 
  }

  if (_offlineStartTime == 0) {
    _offlineStartTime = millis(); 
    Serial.println("[WiFi] WARNING: Connection lost! Waiting for auto-reconnect...");
  }

  // Якщо немає мережі 1 хвилину - рестарт
  if (millis() - _offlineStartTime > 60000) {
    Serial.println("[WiFi] CRITICAL ERROR: Offline for 1 mins. Rebooting...");
    delay(1000);
    ESP.restart(); 
  }
}