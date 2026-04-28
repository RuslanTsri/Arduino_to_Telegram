#ifndef TELEGRAM_SERVICE_H
#define TELEGRAM_SERVICE_H

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Розширена система станів для нижньої клавіатури
enum BotState {
    STATE_IDLE,
    STATE_WAITING_ZONE_FOR_STATUS,
    STATE_WAITING_ZONE_FOR_SETTINGS,
    STATE_WAITING_PARAM_CHOICE,
    STATE_WAITING_PARAM_VALUE
};

class TelegramService {
private:
    WiFiClientSecure secured_client;
    UniversalTelegramBot* bot;
    bool isMaster;
    unsigned long lastTimeBotRan;
    int botRequestDelay = 1000;

    // Змінні для збереження вибору
    BotState currentState = STATE_IDLE;
    int editDeviceId = -1;
    String editParam = "";

    void handleMessages(int numNewMessages);
    
    // Генератори нижніх клавіатур
    String getMainKeyboard();
    String getDevicesKeyboard();
    String getParamsKeyboard();
    String getCancelKeyboard();

public:
    TelegramService();
    void begin(bool deployBot);
    void loop();
    void sendTextMessage(String text);
};

extern TelegramService telegram;

#endif