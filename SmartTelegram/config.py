import os
from dotenv import load_dotenv

load_dotenv()

BOT_TOKEN = os.getenv("BOT_TOKEN")
if not BOT_TOKEN:
    raise ValueError("❌ BOT_TOKEN не знайдено!")

allowed_users_str = os.getenv("ALLOWED_USERS", "")
ALLOWED_USERS = [int(uid.strip()) for uid in allowed_users_str.split(",") if uid.strip()]

MQTT_BROKER = "broker.emqx.io"
MQTT_PORT = 1883

# Тільки ID та топіки. Бот сам спитає ім'я у плати.
ESP_DEVICES = {
    1: {"base_topic": "ipz234_tsri/zone/1"},
    2: {"base_topic": "ipz234_tsri/zone/2"},
    3: {"base_topic": "ipz234_tsri/zone/3"},
}