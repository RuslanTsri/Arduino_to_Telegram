#include "WifiController.h"
#include <Arduino.h>

int WifiController::getSignalStrength() {
    return WiFi.RSSI();
}

bool WifiController::pingHost(const char* host, uint16_t port) {
    WiFiClient client;
    Serial.print("Pinging ");
    Serial.print(host);
    
    // Намагаємося встановити TCP-з'єднання (таймаут 1 сек)
    if (client.connect(host, port)) {
        client.stop();
        Serial.println(" -> SUCCESS");
        return true;
    }
    Serial.println(" -> FAILED");
    return false;
}

void WifiController::disconnect() {
    Serial.println("Disconnecting WiFi...");
    WiFi.disconnect();
}

void WifiController::connectToLast() {
    Serial.println("Attempting to connect to last known network...");
    WiFi.begin(); // Без аргументів - бере з пам'яті
}

void WifiController::printDiagnostics() {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("--- WiFi Diagnostics ---");
        Serial.print("SSID: "); Serial.println(WiFi.SSID());
        Serial.print("IP: "); Serial.println(WiFi.localIP());
        Serial.print("RSSI: "); Serial.print(getSignalStrength()); Serial.println(" dBm");
        Serial.print("Gateway: "); Serial.println(WiFi.gatewayIP());
        Serial.println("------------------------");
    } else {
        Serial.println("WiFi not connected. Diagnostics unavailable.");
    }
}