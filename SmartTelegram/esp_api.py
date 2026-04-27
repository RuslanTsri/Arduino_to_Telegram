import asyncio
import json
import logging
from config import ESP_DEVICES, MQTT_BROKER, MQTT_PORT, ALLOWED_USERS
import paho.mqtt.client as mqtt

# Налаштування логування
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

# Об'єкт бота буде передано сюди з main.py пізніше
tg_bot = None

try:
    from paho.mqtt.enums import CallbackAPIVersion

    client = mqtt.Client(CallbackAPIVersion.VERSION1)
except ImportError:
    client = mqtt.Client()

pending_requests = {}


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        logging.info("🌐 [MQTT] Бот успішно підключився до брокера!")
        for dev_id, info in ESP_DEVICES.items():
            # Підписуємось на статус (для відповідей на кнопки)
            status_topic = f"{info['base_topic']}/status"
            client.subscribe(status_topic)

            # Підписуємось на алярми (для автоматичних сповіщень)
            alarm_topic = f"{info['base_topic']}/alarm"
            client.subscribe(alarm_topic)

            logging.info(f"📡 [MQTT] Слухаю пристрій №{dev_id}: {status_topic} та {alarm_topic}")
    else:
        logging.error(f"❌ [MQTT] Помилка підключення! Код: {rc}")


def on_message(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload.decode('utf-8', errors='ignore')

    # ВИПРАВЛЕННЯ: Використовуємо глобальний main_loop
    global main_loop, tg_bot

    for dev_id, info in ESP_DEVICES.items():
        # 1. ОБРОБКА СТАТУСУ
        if topic == f"{info['base_topic']}/status":
            if dev_id in pending_requests:
                fut_data = pending_requests[dev_id]
                if not fut_data['future'].done():
                    # Передаємо результат у правильний потік
                    fut_data['loop'].call_soon_threadsafe(fut_data['future'].set_result, payload)

        # 2. ОБРОБКА АЛЯРМУ
        elif topic == f"{info['base_topic']}/alarm":
            logging.warning(f"🚨 [MQTT] ОТРИМАНО АЛЯРМ: {payload}")
            if tg_bot and main_loop:
                for user_id in ALLOWED_USERS:
                    # ВИПРАВЛЕННЯ: run_coroutine_threadsafe потребує чіткого main_loop
                    asyncio.run_coroutine_threadsafe(
                        tg_bot.send_message(user_id, payload),
                        main_loop
                    )


client.on_connect = on_connect
client.on_message = on_message


def start_mqtt(bot_instance):
    global tg_bot, main_loop
    tg_bot = bot_instance

    main_loop = asyncio.get_event_loop()

    logging.info(f"🚀 [MQTT] Запуск на {MQTT_BROKER}...")
    client.connect(MQTT_BROKER, MQTT_PORT, 60)
    client.loop_start()

async def fetch_device_mqtt(device_id: int, info: dict) -> str:
    """Відправляє команду і чекає відповідь через хмару"""
    loop = asyncio.get_running_loop()
    fut = loop.create_future()
    pending_requests[device_id] = {'loop': loop, 'future': fut}

    cmd_topic = f"{info['base_topic']}/cmd"
    logging.info(f"📤 [MQTT] ВІДПРАВЛЯЮ запит 'status' у топік: {cmd_topic}")
    client.publish(cmd_topic, "status")

    try:
        # ОСЬ ЦЕЙ РЯДОК БУВ ПРОПУЩЕНИЙ:
        raw_response = await asyncio.wait_for(fut, timeout=4.0)

        try:
            data = json.loads(raw_response)
            # Використовуємо .get(), щоб не вилетіти, якщо поля немає
            temp = data.get('temp', 'N/A')
            name = data.get('name', f"Зона {device_id}")

            # Якщо датчик видає помилку, ЕСП може прислати null або "error"
            if temp == "error" or temp is None:
                temp = "⚠️ Помилка датчика"

            return f"✅ {name}\n🌡 Темп: {temp}°C\n⚙️ Ліміти: [{data.get('min', '?')} - {data.get('max', '?')}]"
        except json.JSONDecodeError:
            # Якщо прийшов не JSON, а просто текст
            return f"💬 Відповідь: {raw_response}"

    except asyncio.TimeoutError:
        logging.warning(f"⏰ [MQTT] ТАЙМАУТ: Пристрій №{device_id} не відповів.")
        return f"❌ Пристрій №{device_id}: Немає зв'язку (Offline)"
    finally:
        pending_requests.pop(device_id, None)


async def get_all_status() -> str:
    tasks = [fetch_device_mqtt(d_id, info) for d_id, info in ESP_DEVICES.items()]
    results = await asyncio.gather(*tasks)
    return "📊 СТАТУС ЛІНІЇ:\n\n" + "\n\n".join(results)


async def get_device_status(device_id: int) -> str:
    info = ESP_DEVICES.get(device_id)
    return await fetch_device_mqtt(device_id, info) if info else "Не знайдено"


async def update_device_param(device_id: int, param: str, value: str) -> bool:
    info = ESP_DEVICES.get(device_id)
    if not info: return False
    # Відправляємо команду (наприклад setmax 85)
    client.publish(f"{info['base_topic']}/cmd", f"set{param} {value}")
    await asyncio.sleep(0.5)  # Пауза на оновлення в залізі
    return True