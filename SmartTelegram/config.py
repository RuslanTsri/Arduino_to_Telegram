import os
from dotenv import load_dotenv

# Завантажуємо змінні з файлу .env у пам'ять
load_dotenv()

# 1. Отримуємо токен
BOT_TOKEN = os.getenv("BOT_TOKEN")
if not BOT_TOKEN:
    raise ValueError("❌ Помилка: BOT_TOKEN не знайдено у файлі .env!")


allowed_users_str = os.getenv("ALLOWED_USERS", "")
if not allowed_users_str:
    raise ValueError("❌ Помилка: ALLOWED_USERS не знайдено у файлі .env!")

ALLOWED_USERS = [int(uid.strip()) for uid in allowed_users_str.split(",") if uid.strip()]

# 3. Наші ардуінки залишаються тут, бо це не секретна інформація,
# а швидше "карта" нашої мережі.
ESP_DEVICES = {
    1: {"name": "Ванна ГАРЯЧА", "ip": "192.168.1.101"},
    2: {"name": "Ванна ХОЛОДНА", "ip": "192.168.1.102"},
    3: {"name": "Сушка №1", "ip": "192.168.1.103"},
}