import asyncio
import logging
from aiogram import Bot, Dispatcher, F, BaseMiddleware
from aiogram.types import Message, CallbackQuery
from aiogram.filters import CommandStart
from aiogram.fsm.context import FSMContext
from aiogram import BaseMiddleware
from typing import Callable, Dict, Any, Awaitable
from aiogram.types import TelegramObject

import config
import keyboards as kb
import esp_api
from states import ParamEditState

# --- МІДЛВАР ДЛЯ ПЕРЕВІРКИ ДОСТУПУ (Пункт 1) ---
class SecurityMiddleware(BaseMiddleware):
    async def __call__(
        self,
        handler: Callable[[TelegramObject, Dict[str, Any]], Awaitable[Any]],
        event: TelegramObject,
        data: Dict[str, Any]
    ) -> Any:
        # Перевіряємо ID (працює і для повідомлень, і для натискання кнопок)
        if event.from_user.id not in config.ALLOWED_USERS:
            logging.warning(f"Спроба доступу від чужого ID: {event.from_user.id}")
            return # Ігноруємо
        return await handler(event, data)
# Ініціалізація
logging.basicConfig(level=logging.INFO)
bot = Bot(token=config.BOT_TOKEN)
dp = Dispatcher()





# --- ГОЛОВНЕ МЕНЮ (Пункт 2) ---
@dp.message(CommandStart())
async def cmd_start(message: Message, state: FSMContext):
    await state.clear()
    await message.answer("Головне меню лінії:", reply_markup=kb.get_main_menu())


@dp.callback_query(F.data == "cmd_main_menu")
async def back_to_main(callback: CallbackQuery, state: FSMContext):
    await state.clear()
    await callback.message.edit_text("Головне меню лінії:", reply_markup=kb.get_main_menu())


# --- С1. ПЕРЕВІРИТИ ВСІ (Пункт 3) ---
@dp.callback_query(F.data == "cmd_check_all")
async def check_all(callback: CallbackQuery):
    status_text = await esp_api.get_all_status()
    await callback.message.edit_text(status_text, reply_markup=kb.get_cancel_menu())


# --- С2. ВИБІРКОВО (Пункт 4) ---
@dp.callback_query(F.data == "cmd_selective")
async def selective_menu(callback: CallbackQuery):
    await callback.message.edit_text("Оберіть зону для моніторингу:", reply_markup=kb.get_devices_menu("sel"))


@dp.callback_query(F.data.startswith("sel_"))
async def selective_show_device(callback: CallbackQuery):
    device_id = int(callback.data.split("_")[1])
    status_text = await esp_api.get_device_status(device_id)

    # Кнопка повернення до списку
    builder = kb.InlineKeyboardBuilder()
    builder.button(text="🔙 Назад до списку", callback_data="cmd_selective")
    await callback.message.edit_text(status_text, reply_markup=builder.as_markup())


# --- С3. НАЛАШТУВАННЯ (Пункт 5 та 6) ---
@dp.callback_query(F.data == "cmd_settings")
async def settings_menu(callback: CallbackQuery):
    await callback.message.edit_text("Оберіть зону для налаштування:", reply_markup=kb.get_devices_menu("set"))


# --- Продовження С3 ч2 (Пункт 7: Вибір параметра) ---
@dp.callback_query(F.data.startswith("set_"))
async def settings_device_menu(callback: CallbackQuery):
    device_id = int(callback.data.split("_")[1])
    name = config.ESP_DEVICES[device_id]["name"]
    await callback.message.edit_text(f"Налаштування: {name}\nЩо будемо міняти?",
                                     reply_markup=kb.get_device_params_menu(device_id))


# --- Продовження С3 ч3 (Пункт 8: Ввід параметра та відміна) ---
@dp.callback_query(F.data.startswith("editparam_"))
async def ask_for_param_value(callback: CallbackQuery, state: FSMContext):
    _, device_id, param = callback.data.split("_")

    # Зберігаємо в пам'ять станів (FSM), яку плату і який параметр ми міняємо
    await state.update_data(device_id=int(device_id), param=param)
    await state.set_state(ParamEditState.waiting_for_value)

    await callback.message.edit_text(f"Введіть нове значення для [{param.upper()}]:", reply_markup=kb.get_cancel_menu())


@dp.callback_query(F.data == "cmd_cancel_input")
async def cancel_input(callback: CallbackQuery, state: FSMContext):
    await state.clear()  # Скидаємо стан очікування вводу
    await callback.message.edit_text("Дію скасовано.", reply_markup=kb.get_main_menu())


@dp.message(ParamEditState.waiting_for_value)
async def process_new_param_value(message: Message, state: FSMContext):
    # Користувач ввів текст
    new_value = message.text
    data = await state.get_data()
    device_id = data["device_id"]
    param = data["param"]

    # 1. Відправляємо на ESP32 (заглушка)
    await esp_api.update_device_param(device_id, param, new_value)

    # 2. Робимо запит статусу, щоб переконатися, що зміни застосовано
    status_text = await esp_api.get_device_status(device_id)

    await state.clear()
    await message.answer(f"✅ Зміни застосовано!\n\n{status_text}", reply_markup=kb.get_main_menu())


# --- ЗАПУСК БОТА ---
async def main():
    await dp.start_polling(bot)


if __name__ == "__main__":
    asyncio.run(main())