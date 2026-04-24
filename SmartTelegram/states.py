from aiogram.fsm.state import State, StatesGroup

class ParamEditState(StatesGroup):
    waiting_for_value = State()