#include "m6502_opcode_handlers.h"
#include "m6502_chip_instance.h"
#include "m6502_addressing_handlers.h"
#include "m6502_memory_utils.h"

//TODO: Добавить обработку флагов в команды

#define SKIP_UNTIL_ZERO(n)\
  if ((n) > 0)            \
  {                       \
    (n)--;                \
    return M6502_WAIT;    \
  }

typedef union
{
  uint8_t bytes[2];
  uint16_t word;
} RawOperands;

RawOperands get_raw_operands(M6502_HANDLE handle, uint8_t instrsize)
{
  RawOperands operands;
  for (uint8_t i = 0; (i < instrsize-1) && (i < 2); i++)
  {
    operands.bytes[i] = read_byte(handle, handle->state.pc + i + 1);
  }
  return operands;
}

/**
 * Функция, получающая данные для арифметической операции.
 * Будет стараться получить их из непосредственного операнда или
 * из памяти. Если операнд implied, выйдет с ненулевым статус-кодом
 */
int _ariphmetic_get_data(const M6502_HANDLE handle,
                         const struct m6502_Operands* operands,
                         uint8_t* accumulator)
{

  uint16_t real_addr;
  switch(operands->type)
  {
    case RESULT_DATA:
      *accumulator = operands->content.data;
      break;
    case RESULT_ADDR:
      *accumulator = handle->bus.read(handle->bus.handle,
                                      operands->content.address);
      break;
    case RESULT_IND:
      real_addr = read_word(handle, operands->content.address);
      *accumulator = read_byte(handle, real_addr);
      break;
    case RESULT_A:
      *accumulator = handle->state.a;
      break;
    case RESULT_IMPL:
      return M6502_ERR;
  }
  return M6502_OK;
}


// Структура, нужная только для одной фунции - хранит и
// информацию о выбраных операндах и дополнительные такты
struct ExtendedOperands
{
  struct m6502_Operands operands;
  uint8_t extra_cycles;
};

/**
 * Выполняет выборку операндов через обработчик адресации, но не выполняет
 * обращений к шине для их разрешения
 */
struct ExtendedOperands _get_unresolved_operands(
    const M6502_HANDLE handle,
    const struct m6502_OpCodeDesc* desc
    )
{
  ADDRESSING_HANDLER addressing_handler = m6502_get_addressing_handler(desc->addressmode);
  uint8_t extra_cycles;
  struct m6502_Operands operands = addressing_handler(
      handle,
      get_raw_operands(handle, desc->instrsize).word,
      &extra_cycles
      );
  struct ExtendedOperands result = {operands, extra_cycles};
  return result;
}

/**
 * Структура, аккумулирующая информацию о результате выборки операндов
 * из памяти. Содержит информацию о выбранном операнде, о потраченных
 * на выборку дополнительных циклов (появляются в некоторых видах адресации)
 * и о коде возврата из функции. Предполагается, что передается через стек
 */
struct ParsedData
{
  uint8_t data;
  uint8_t extra_cycles;
  int return_code;
};

/**
 * Производит выборку операндов из памяти и возвращает их в структуре вместе
 * с кодом возврата и дополнительными циклами, потраченными функцией адресации.
 * Функция годится для тех случаев, когда операнд в виде числа получить нужно
 * в любом случае
 */
struct ParsedData _parse_data(const M6502_HANDLE handle,
                               const struct m6502_OpCodeDesc* desc)
{
  struct ExtendedOperands operands = _get_unresolved_operands(handle, desc);
  uint8_t data;
  int ret_code = _ariphmetic_get_data(handle, &(operands.operands), &data);
  struct ParsedData result = {data, operands.extra_cycles, 0};
  if (ret_code != M6502_OK)
  {
    result.return_code = M6502_ERR;
  }
  return result;
}

// Очень некрасивая прембула, которая выполняет набор необходимых действий
// для выполнения почти любой арифметической операции в эмуляторе:
// Подсчитывает такты до начала выполнения операции, парсит операнды функции
// и выдает ошибку, если операнд implied, поскольку для арифметической операции
// это в основном не имеет смысла, а остальные случаи можно обработать руками
#define ARIPHMETIC_PREAMBLE(handle, desc, parsed_data_reciever)            \
  SKIP_UNTIL_ZERO((handle)->cycles_remaining);                             \
  struct ParsedData (parsed_data_reciever) = _parse_data((handle), (desc));\
  if ((parsed_data_reciever).return_code != 0)                             \
  {                                                                        \
    return M6502_ERR;                                                      \
  }

int handle_NOP(M6502_HANDLE handle, const struct m6502_OpCodeDesc* desc)
{
  SKIP_UNTIL_ZERO(handle->cycles_remaining);
  handle->state.pc += desc->instrsize;
  handle->cycles_remaining = desc->minrequiredcycles;
  return 0;
}

/**
 * Функция, устанавливающая флаги zero и negative
 * по переданному в нее числу. Есть надежда, что
 * функция будет заинлайнена
 */
void _set_z_n(M6502_HANDLE handle, uint8_t result)
{
  // Сбрасываем значения флагов
  handle->state.sr &= ~(M6502_Z | M6502_N);
  if (result == 0)
  {
    handle->state.sr |= M6502_Z;
  }
  if (result&0x80)
  {
    handle->state.sr |= M6502_N;
  }
}

/**
 * Прибавляет операнд к аккумулятору вместе с флагом переноса
 */
int handle_ADC(M6502_HANDLE handle, const struct m6502_OpCodeDesc* desc)
{
  ARIPHMETIC_PREAMBLE(handle, desc, parsed);
  // Сбросим все флаги, которые меняются
  handle->state.sr &= ~(M6502_C | M6502_Z | M6502_N | M6502_V);
  // определяем carry flag
  uint8_t old_c_flag = M6502_GET_C(handle->state.sr);
  if (0xff - parsed.data - old_c_flag < handle->state.a)
  {
    handle->state.sr |= M6502_C;
  }

  uint8_t result = handle->state.a + parsed.data + old_c_flag;
  _set_z_n(handle, handle->state.a);
  // Проверяем переполнение. Берется уравнение из спецификации
  if ((result^handle->state.a)&(result^parsed.data)&0x80)
  {
    handle->state.sr |= M6502_V;
  }
  handle->state.a = result;
  handle->cycles_remaining = desc->minrequiredcycles + parsed.extra_cycles;
  return M6502_OK;
}

/**
 * Вычитает операнд из аккумулятора с учетом флага переноса
 */
int handle_SBC(M6502_HANDLE handle, const struct m6502_OpCodeDesc* desc)
{
  ARIPHMETIC_PREAMBLE(handle, desc, parsed);
  handle->state.sr &= ~(M6502_C | M6502_Z | M6502_N | M6502_V);
  handle->state.a = handle->state.a + ~parsed.data + M6502_GET_C(handle->state.sr);
  _set_z_n(handle, handle->state.a);
  handle->cycles_remaining = desc->minrequiredcycles + parsed.extra_cycles;
  return M6502_OK;
}

int handle_INY(M6502_HANDLE handle, const struct m6502_OpCodeDesc* desc)
{
  SKIP_UNTIL_ZERO(handle->cycles_remaining)
  handle->state.y++;
  handle->cycles_remaining = desc->minrequiredcycles;
  return M6502_OK;
}

int handle_DEY(M6502_HANDLE handle, const struct m6502_OpCodeDesc* desc)
{
  SKIP_UNTIL_ZERO(handle->cycles_remaining)
  handle->state.y--;
  handle->cycles_remaining = desc->minrequiredcycles;
  return M6502_OK;
}

int handle_INX(M6502_HANDLE handle, const struct m6502_OpCodeDesc* desc)
{
  SKIP_UNTIL_ZERO(handle->cycles_remaining)
  handle->state.x++;
  handle->cycles_remaining = desc->minrequiredcycles;
  return M6502_OK;
}

int handle_DEX(M6502_HANDLE handle, const struct m6502_OpCodeDesc* desc)
{
  SKIP_UNTIL_ZERO(handle->cycles_remaining)
  handle->state.x--;
  handle->cycles_remaining = desc->minrequiredcycles;
  return M6502_OK;
}

int handle_AND(M6502_HANDLE handle, const struct m6502_OpCodeDesc* desc)
{
  ARIPHMETIC_PREAMBLE(handle, desc, parsed);
  handle->state.a &= parsed.data;
  handle->cycles_remaining = desc->minrequiredcycles + parsed.extra_cycles;
  return M6502_OK;
}

int handle_ORA(M6502_HANDLE handle, const struct m6502_OpCodeDesc* desc)
{
  ARIPHMETIC_PREAMBLE(handle, desc, parsed);
  handle->state.a |= parsed.data;
  handle->cycles_remaining = desc->minrequiredcycles + parsed.extra_cycles;
  return M6502_OK;
}

int handle_EOR(M6502_HANDLE handle, const struct m6502_OpCodeDesc* desc)
{
  ARIPHMETIC_PREAMBLE(handle, desc, parsed);
  handle->state.a ^= parsed.data;
  handle->cycles_remaining = desc->minrequiredcycles + parsed.extra_cycles;
  return M6502_OK;
}

// В микроконтроллере есть особенность, что иногда он вместо
// одной записи в память выполняет 2 - записывает старое значение
// памяти, а затем на его место сразу новое
void _read_modify_write(M6502_HANDLE handle, uint16_t addr,
                        uint8_t original, uint8_t new)
{
  write_byte(handle, addr, original);
  write_byte(handle, addr, new);
}

// Я правда не горжусь этим шаблоном, но поскольку я не хочу один и тот же код
// дублировать 4 раза с минимальными изменениями, я выбираю страдания при
// отладке, но одну точку изменений, чем копипасту
#define IMPLEMENT_SHIFT_HANDLER(mnem, func)\
int handle_##mnem(M6502_HANDLE handle, const struct m6502_OpCodeDesc* desc)   \
{                                                                             \
  SKIP_UNTIL_ZERO(handle->cycles_remaining)                                   \
  if (desc->addressmode == AM_A)                                              \
  {                                                                           \
    handle->state.a = (func)(handle, handle->state.a);                        \
    return M6502_OK;                                                          \
  }                                                                           \
  struct ExtendedOperands unresop = _get_unresolved_operands(handle, desc);   \
  /* согласно спецификации инструкция может обрабатывать только абсолютные */ \
  /* адреса или zeropage. Поэтому любой другой вид адресации должен        */ \
  /* вызывать ошибку                                                       */ \
  if (unresop.operands.type != RESULT_ADDR)                                   \
  {                                                                           \
    return M6502_ERR;                                                         \
  }                                                                           \
  uint8_t data = read_byte(handle, unresop.operands.content.address);         \
  uint8_t result = (func)(handle, data);                                      \
  /* Согласно спецификации микроконтроллер запишет в ячейку памяти */         \
  /* сначала исходное число, а уже потом измененное                */         \
  _read_modify_write(handle, unresop.operands.content.address, data, result); \
  return M6502_OK;                                                            \
}                                                                             \

uint8_t _perform_ASL(M6502_HANDLE handle, uint8_t data)
{
  return data << 1;
}

IMPLEMENT_SHIFT_HANDLER(ASL, _perform_ASL);

uint8_t _perform_LSR(M6502_HANDLE handle, uint8_t data)
{
  return data >> 1;
}

IMPLEMENT_SHIFT_HANDLER(LSR, _perform_LSR);

uint8_t _perform_ROL(M6502_HANDLE handle, uint8_t data)
{
  uint8_t sr = handle->state.sr;
  uint8_t old_c_flag = M6502_GET_C(sr);
  // Обновляем флаг carry
  handle->state.sr = (sr&(~M6502_C)) | (M6502_C*((data&0x80)>>7));
  return (data << 1) + old_c_flag;
}

IMPLEMENT_SHIFT_HANDLER(ROL, _perform_ROL);

uint8_t _perform_ROR(M6502_HANDLE handle, uint8_t data)
{
  uint8_t sr = handle->state.sr;
  uint8_t old_c_flag = M6502_GET_C(sr);
  // Обновляем флаг carry
  handle->state.sr = (sr&(~M6502_C)) | (M6502_C*(data&0x01));
  return (data >> 1) + (old_c_flag << 7);
}

IMPLEMENT_SHIFT_HANDLER(ROR, _perform_ROR);

// Макрос, генерирующий очередной выход функции по названию мнемоники.
// Работает в предположении, что каждая мнемоника обрабатывается фунцией
// handle_(mnemonic_name), e.g. handle_ADC, handle_LDA, ...
#define ADD_HANDLER(mnemonic_name)\
  case MNEM_##mnemonic_name:      \
    return handle_##mnemonic_name;\
    break;

OPCODE_HANDLER m6502_get_opcode_handler(enum m6502_Mnemonic mnemonic)
{
  // Сейчас я использую максимально простую реализацию
  switch(mnemonic)
  {
    ADD_HANDLER(ADC);
    ADD_HANDLER(SBC);
    ADD_HANDLER(INX);
    ADD_HANDLER(DEX);
    ADD_HANDLER(INY);
    ADD_HANDLER(DEY);
    ADD_HANDLER(NOP);
    ADD_HANDLER(AND);
    ADD_HANDLER(ORA);
    ADD_HANDLER(EOR);
    ADD_HANDLER(ASL);
    ADD_HANDLER(LSR);
    ADD_HANDLER(ROL);
    ADD_HANDLER(ROR);
    default:
      return handle_NOP;
  }
}
