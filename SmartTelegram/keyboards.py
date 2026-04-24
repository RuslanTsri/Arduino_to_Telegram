from aiogram.types import InlineKeyboardMarkup, InlineKeyboardButton
from aiogram.utils.keyboard import InlineKeyboardBuilder
from config import ESP_DEVICES

def get_main_menu() -> InlineKeyboardMarkup:
    builder = InlineKeyboardBuilder()
    builder.button(text="📋 Перевірити всі", callback_data="cmd_check_all")
    builder.button(text="🎯 Вибірково", callback_data="cmd_selective")
    builder.button(text="⚙️ Налаштування", callback_data="cmd_settings")
    builder.adjust(1) # По одній кнопці в ряд
    return builder.as_markup()

def get_devices_menu(action_prefix: str) -> InlineKeyboardMarkup:
    # action_prefix буде "sel" (вибірково) або "set" (налаштування)
    builder = InlineKeyboardBuilder()
    for dev_id, dev_info in ESP_DEVICES.items():
        builder.button(text=dev_info["name"], callback_data=f"{action_prefix}_{dev_id}")
    builder.button(text="🔙 Назад", callback_data="cmd_main_menu")
    builder.adjust(1)
    return builder.as_markup()

def get_device_params_menu(device_id: int) -> InlineKeyboardMarkup:
    builder = InlineKeyboardBuilder()
    builder.button(text="🔴 Максимальна темп.", callback_data=f"editparam_{device_id}_max")
    builder.button(text="🔵 Мінімальна темп.", callback_data=f"editparam_{device_id}_min")
    builder.button(text="📝 Змінити ім'я", callback_data=f"editparam_{device_id}_name")
    builder.button(text="🔙 Назад", callback_data="cmd_settings")
    builder.adjust(1)
    return builder.as_markup()

def get_cancel_menu() -> InlineKeyboardMarkup:
    builder = InlineKeyboardBuilder()
    builder.button(text="❌ Відміна", callback_data="cmd_cancel_input")
    return builder.as_markup()