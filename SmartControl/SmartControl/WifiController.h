#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include <WiFi.h>

class WifiController {
  public:
    // Отримати силу сигналу в dBm (допоможе зрозуміти, як далеко роутер)
    int getSignalStrength();
    
    // Перевірка доступності конкретного хоста (спрощений пінг через TCP)
    bool pingHost(const char* host, uint16_t port = 80);
    
    // Відключення
    void disconnect();
    
    // Підключення до останньої збереженої мережі (з пам'яті ESP)
    void connectToLast();

    // Вивести інформацію про поточне з'єднання в консоль
    void printDiagnostics();
};

#endif