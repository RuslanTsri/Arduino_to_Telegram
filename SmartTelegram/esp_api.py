from config import ESP_DEVICES

async def get_all_status() -> str:
    # Імітація запиту до всіх плат (Пункт 3)
    return "📊 СТАТУС УСІХ ПЛАТ:\n\n1. Гаряча: 80°C\n2. Холодна: 20°C\n3. Сушка: 95°C"

async def get_device_status(device_id: int) -> str:
    # Імітація запиту /status до конкретної плати (Пункт 4)
    name = ESP_DEVICES[device_id]["name"]
    return f"📍 Статус {name}:\n🌡 Температура: 82.5°C\n⚙️ Ліміти: [50.0 - 85.0]"

async def update_device_param(device_id: int, param: str, value: str) -> bool:
    # Імітація відправки команди /setmax, /setmin на плату (Пункт 8)
    print(f"[API] На плату {device_id} відправлено {param}={value}")
    return True