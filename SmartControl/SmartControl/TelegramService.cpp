#include "TelegramService.h"
#include "Config.h"           
#include "MqttService.h" 

extern MqttService* globalMqttInstance;
TelegramService telegram;

TelegramService::TelegramService() {
    isMaster = false;
    lastTimeBotRan = 0;
}

void TelegramService::begin(bool deployBot) {
    isMaster = deployBot;
    if (!isMaster) return;

    Serial.println("[TG] Ініціалізація Telegram (Reply Keyboard)...");
    secured_client.setInsecure(); 
    bot = new UniversalTelegramBot(BOT_TOKEN, secured_client);
}

void TelegramService::loop() {
    if (!isMaster) return;

    if (millis() - lastTimeBotRan > botRequestDelay) {
        int numNewMessages = bot->getUpdates(bot->last_message_received + 1);
        while (numNewMessages) {
            handleMessages(numNewMessages);
            numNewMessages = bot->getUpdates(bot->last_message_received + 1);
        }
        lastTimeBotRan = millis();
    }
}

void TelegramService::sendTextMessage(String text) {
    if (isMaster) {
        bot->sendMessage(CHAT_ID, text, ""); // Змінено на CHAT_ID
    }
}

// Генератори клавіатур
String TelegramService::getMainKeyboard() {
    return "[[{\"text\":\"📋 Перевірити всі\"}], [{\"text\":\"🎯 Вибірково\"}, {\"text\":\"⚙️ Налаштування\"}]]";
}

String TelegramService::getDevicesKeyboard() {
    String kb = "[";
    for (int i = 1; i <= TOTAL_ZONES; i++) {
        kb += "[{\"text\":\"📦 Пристрій №" + String(i) + "\"}],";
    }
    kb += "[{\"text\":\"🔙 Назад\"}]]";
    return kb;
}

String TelegramService::getParamsKeyboard() {
    return "[[{\"text\":\"🔴 Max Temp\"}, {\"text\":\"🔵 Min Temp\"}], [{\"text\":\"📝 Назва\"}], [{\"text\":\"🔙 Назад\"}]]";
}

String TelegramService::getCancelKeyboard() {
    return "[[{\"text\":\"❌ Відміна\"}]]";
}

void TelegramService::handleMessages(int numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
        String chatId = String(bot->messages[i].chat_id);
        String text = bot->messages[i].text;
        
        // Змінено на CHAT_ID
        if (chatId != String(CHAT_ID)) continue; 

        if (text == "🔙 Назад" || text == "❌ Відміна" || text == "/start") {
            currentState = STATE_IDLE;
            // ВИПРАВЛЕНО: замість 0 передаємо порожні лапки ""
            bot->sendMessageWithReplyKeyboard(chatId, "Головне меню:", "", getMainKeyboard(), true);
            continue;
        }

        if (currentState == STATE_IDLE) {
            if (text == "📋 Перевірити всі") {
                bot->sendMessage(chatId, "⏳ Запитую статус всіх пристроїв...", "");
                if (globalMqttInstance) {
                    for (int i = 1; i <= TOTAL_ZONES; i++) {
                        globalMqttInstance->publishCommand("ipz234_tsri/zone/" + String(i) + "/cmd", "status");
                    }
                }
            } 
            else if (text == "🎯 Вибірково") {
                currentState = STATE_WAITING_ZONE_FOR_STATUS;
                // ВИПРАВЛЕНО: замість 0 передаємо порожні лапки ""
                bot->sendMessageWithReplyKeyboard(chatId, "Оберіть зону для перевірки:", "", getDevicesKeyboard(), true);
            } 
            else if (text == "⚙️ Налаштування") {
                currentState = STATE_WAITING_ZONE_FOR_SETTINGS;
                // ВИПРАВЛЕНО: замість 0 передаємо порожні лапки ""
                bot->sendMessageWithReplyKeyboard(chatId, "Яку зону налаштовуємо?", "", getDevicesKeyboard(), true);
            }
            continue;
        }

        if (currentState == STATE_WAITING_ZONE_FOR_STATUS) {
          if (text.startsWith("📦 Пристрій №")) {
           // Знаходимо позицію символу №. Він займає 3 байти в UTF-8.
            int pos = text.indexOf("№");
            int devId = text.substring(pos + 3).toInt(); // Беремо все після №
        
            Serial.printf("[TG] Запит статусу для зони: %d\n", devId);
            bot->sendMessage(chatId, "⏳ Запитую статус пристрою №" + String(devId) + "...", "");
            if (globalMqttInstance) {
            globalMqttInstance->publishCommand("ipz234_tsri/zone/" + String(devId) + "/cmd", "status");
          }
        }
        continue;
       }

        if (currentState == STATE_WAITING_ZONE_FOR_SETTINGS) {
    if (text.startsWith("📦 Пристрій №")) {
        int pos = text.indexOf("№");
        editDeviceId = text.substring(pos + 3).toInt(); // Беремо все після №
        
        Serial.printf("[TG] Налаштування зони: %d\n", editDeviceId);
        currentState = STATE_WAITING_PARAM_CHOICE;
        bot->sendMessageWithReplyKeyboard(chatId, "Що змінюємо для пристрою №" + String(editDeviceId) + "?", "", getParamsKeyboard(), true);
    }
    continue;
}

        if (currentState == STATE_WAITING_PARAM_CHOICE) {
            if (text == "🔴 Max Temp") editParam = "max";
            else if (text == "🔵 Min Temp") editParam = "min";
            else if (text == "📝 Назва") editParam = "name";
            else continue;

            currentState = STATE_WAITING_PARAM_VALUE;
            // ВИПРАВЛЕНО: замість 0 передаємо порожні лапки ""
            bot->sendMessageWithReplyKeyboard(chatId, "Введіть нове значення для " + text + ":", "", getCancelKeyboard(), true);
            continue;
        }

        if (currentState == STATE_WAITING_PARAM_VALUE) {
            bot->sendMessage(chatId, "⏳ Відправка змін до ESP...", "");
            
            String topic = "ipz234_tsri/zone/" + String(editDeviceId) + "/cmd";
            String cmd = "set" + editParam + " " + text;
            
            if (globalMqttInstance) {
                globalMqttInstance->publishCommand(topic, cmd); 
            }
            
            currentState = STATE_IDLE;
            // ВИПРАВЛЕНО: замість 0 передаємо порожні лапки ""
            bot->sendMessageWithReplyKeyboard(chatId, "✅ Команду надіслано!", "", getMainKeyboard(), true);
            continue;
        }
    }
}