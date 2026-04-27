from aiogram.types import InlineKeyboardMarkup
from aiogram.utils.keyboard import InlineKeyboardBuilder
from config import ESP_DEVICES

def get_main_menu():
    builder = InlineKeyboardBuilder()
    builder.button(text="📋 Перевірити всі", callback_data="cmd_check_all")
    builder.button(text="🎯 Вибірково", callback_data="cmd_selective")
    builder.button(text="⚙️ Налаштування", callback_data="cmd_settings")
    return builder.adjust(1).as_markup()

def get_devices_menu(action_prefix):
    builder = InlineKeyboardBuilder()
    # Беремо тільки ID з конфігу. Текст кнопки буде універсальним.
    for dev_id in ESP_DEVICES.keys():
        builder.button(text=f"📦 Пристрій №{dev_id}", callback_data=f"{action_prefix}_{dev_id}")
    builder.button(text="🔙 Назад", callback_data="cmd_main_menu")
    return builder.adjust(1).as_markup()

def get_device_params_menu(device_id):
    builder = InlineKeyboardBuilder()
    builder.button(text="🔴 Max Temp", callback_data=f"editparam_{device_id}_max")
    builder.button(text="🔵 Min Temp", callback_data=f"editparam_{device_id}_min")
    builder.button(text="📝 Назва", callback_data=f"editparam_{device_id}_name")
    builder.button(text="🔙 Назад", callback_data="cmd_settings")
    return builder.adjust(1).as_markup()

def get_cancel_menu():
    builder = InlineKeyboardBuilder()
    builder.button(text="❌ Відміна", callback_data="cmd_cancel_input")
    return builder.as_markup()