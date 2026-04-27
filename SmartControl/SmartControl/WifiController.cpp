#include "WifiController.h"
#include <Arduino.h>

int WifiController::getSignalStrength() {
    return WiFi.RSSI();
}

bool WifiController::pingHost(const char* host, uint16_t port) {
    WiFiClient client;
    Serial.print("[Diag] Pinging ");
    Serial.print(host);
    
    if (client.connect(host, port)) {
        client.stop();
        Serial.println(" -> SUCCESS");
        return true;
    }
    Serial.println(" -> FAILED");
    return false;
}

void WifiController::connectToLast() {
    Serial.println("[Diag] Attempting to connect to last known network...");
    WiFi.begin(); 
}

void WifiController::printDiagnostics() {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n--- WiFi Diagnostics ---");
        Serial.print("SSID: "); Serial.println(WiFi.SSID());
        Serial.print("IP: "); Serial.println(WiFi.localIP());
        Serial.print("RSSI: "); Serial.print(getSignalStrength()); Serial.println(" dBm");
        Serial.print("Gateway: "); Serial.println(WiFi.gatewayIP());
        Serial.print("MAC: "); Serial.println(WiFi.macAddress());
        Serial.println("------------------------\n");
    } else {
        Serial.println("[Diag] WiFi not connected. Diagnostics unavailable.");
    }
}