import asyncio
import logging
from aiogram import Bot, Dispatcher, F, BaseMiddleware
from aiogram.types import Message, CallbackQuery, Update
from aiogram.filters import CommandStart
from aiogram.fsm.context import FSMContext
from aiogram.exceptions import TelegramBadRequest
from aiogram.utils.keyboard import InlineKeyboardBuilder

import config
import keyboards as kb
import esp_api
from states import ParamEditState


# --- ФІКС МІДЛВАРУ ---
class SecurityMiddleware(BaseMiddleware):
    async def __call__(self, handler, event, data):
        # В aiogram 3.x користувач приходить у словнику data
        user = data.get("event_from_user")
        if user and user.id not in config.ALLOWED_USERS:
            logging.warning(f"Доступ заборонено користувачу: {user.id}")
            return
        return await handler(event, data)


bot = Bot(token=config.BOT_TOKEN)
dp = Dispatcher()
dp.update.middleware(SecurityMiddleware())


# --- ФУНКЦІЯ АНІМАЦІЇ ---
async def show_loading_animation(msg: Message):
    frames = [
        "⏳ Отримання відповіді ESP32 .",
        "⏳ Отримання відповіді ESP32 ..",
        "⏳ Отримання відповіді ESP32 ..."
    ]
    idx = 1  # Починаємо з 1, бо нульовий кадр відправляємо відразу
    while True:
        try:
            await msg.edit_text(frames[idx % len(frames)])
            idx += 1
            await asyncio.sleep(0.8)  # Безпечний інтервал для Telegram
        except asyncio.CancelledError:
            # Спрацьовує, коли ми примусово зупиняємо анімацію
            break
        except TelegramBadRequest:
            # Ігноруємо помилки (наприклад, якщо текст ще не встиг оновитися)
            await asyncio.sleep(0.8)


@dp.message(CommandStart())
async def cmd_start(message: Message, state: FSMContext):
    await state.clear()
    await message.answer("Управління лінією (MQTT):", reply_markup=kb.get_main_menu())


@dp.callback_query(F.data == "cmd_main_menu")
async def back_to_main(callback: CallbackQuery, state: FSMContext):
    await state.clear()
    await callback.message.edit_text("Головне меню:", reply_markup=kb.get_main_menu())


@dp.callback_query(F.data == "cmd_check_all")
async def check_all(callback: CallbackQuery):
    await callback.answer()  # Прибираємо "годинник" на самій кнопці

    # Відправляємо початковий текст і запускаємо анімацію
    await callback.message.edit_text("⏳ Отримання відповіді ESP32 .")
    animation_task = asyncio.create_task(show_loading_animation(callback.message))

    try:
        status_text = await esp_api.get_all_status()
    finally:
        # Зупиняємо анімацію і виводимо результат
        animation_task.cancel()
        await callback.message.edit_text(status_text, reply_markup=kb.get_cancel_menu())


@dp.callback_query(F.data == "cmd_selective")
async def selective_menu(callback: CallbackQuery):
    await callback.message.edit_text("Оберіть зону:", reply_markup=kb.get_devices_menu("sel"))


@dp.callback_query(F.data.startswith("sel_"))
async def selective_show_device(callback: CallbackQuery):
    device_id = int(callback.data.split("_")[1])
    await callback.answer()

    # Анімація
    await callback.message.edit_text("⏳ Отримання відповіді ESP32 .")
    animation_task = asyncio.create_task(show_loading_animation(callback.message))

    try:
        status_text = await esp_api.get_device_status(device_id)
    finally:
        animation_task.cancel()
        # Створюємо кнопку повернення
        builder = InlineKeyboardBuilder()
        builder.button(text="🔙 Назад", callback_data="cmd_selective")
        await callback.message.edit_text(status_text, reply_markup=builder.as_markup())


@dp.callback_query(F.data == "cmd_settings")
async def settings_menu(callback: CallbackQuery):
    await callback.message.edit_text("Що налаштовуємо?", reply_markup=kb.get_devices_menu("set"))


@dp.callback_query(F.data.startswith("set_"))
async def settings_device_menu(callback: CallbackQuery):
    device_id = int(callback.data.split("_")[1])
    await callback.message.edit_text(f"Налаштування пристрою №{device_id}:",
                                     reply_markup=kb.get_device_params_menu(device_id))


@dp.callback_query(F.data.startswith("editparam_"))
async def ask_for_param_value(callback: CallbackQuery, state: FSMContext):
    _, device_id, param = callback.data.split("_")
    await state.update_data(device_id=int(device_id), param=param)
    await state.set_state(ParamEditState.waiting_for_value)
    await callback.message.edit_text(f"Введіть нове значення для {param.upper()}:", reply_markup=kb.get_cancel_menu())


@dp.message(ParamEditState.waiting_for_value)
async def process_new_param_value(message: Message, state: FSMContext):
    data = await state.get_data()

    # Відправляємо нове повідомлення з анімацією
    loading_msg = await message.answer("⏳ Отримання відповіді ESP32 .")
    animation_task = asyncio.create_task(show_loading_animation(loading_msg))

    try:
        await esp_api.update_device_param(data["device_id"], data["param"], message.text)
        # Оновлюємо статус, щоб побачити нову назву чи ліміт
        status_text = await esp_api.get_device_status(data["device_id"])
    finally:
        animation_task.cancel()
        await state.clear()
        await loading_msg.edit_text(f"✅ Зміни відправлено!\n\n{status_text}", reply_markup=kb.get_main_menu())


@dp.callback_query(F.data == "cmd_cancel_input")
async def cancel_input(callback: CallbackQuery, state: FSMContext):
    await state.clear()
    await callback.message.edit_text("Дію скасовано.", reply_markup=kb.get_main_menu())


async def main():
    esp_api.start_mqtt(bot)
    await dp.start_polling(bot)


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    asyncio.run(main())