#include "m6502_opcode_handlers.h"
#include "m6502_chip_instance.h"
#include "m6502_addressing_handlers.h"
#include "m6502_memory_utils.h"

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
  for (uint8_t i = 0; i < instrsize-1; i++)
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
    case RESULT_IMPL:
      return M6502_ERR;
  }
  return 0;
}

int handle_NOP(M6502_HANDLE handle, const struct m6502_OpCodeDesc* desc)
{
  SKIP_UNTIL_ZERO(handle->cycles_remaining);
  handle->state.pc += desc->instrsize;
  handle->cycles_remaining = desc->minrequiredcycles;
  return 0;
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
 * с кодом возврата и дополнительными циклами, потраченными функцией адресации
 */
struct ParsedData _parse_data(const M6502_HANDLE handle,
                               const struct m6502_OpCodeDesc* desc)
{
  ADDRESSING_HANDLER addressing_handler = m6502_get_addressing_handler(desc->addressmode);
  uint8_t extra_cycles;
  struct m6502_Operands operands = addressing_handler(
      handle,
      get_raw_operands(handle, desc->instrsize).word,
      &extra_cycles
      );
  uint8_t data;
  int ret_code = _ariphmetic_get_data(handle, &operands, &data);
  struct ParsedData result = {data, extra_cycles, 0};
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


int handle_ADC(M6502_HANDLE handle, const struct m6502_OpCodeDesc* desc)
{
  ARIPHMETIC_PREAMBLE(handle, desc, parsed);
  handle->state.a += parsed.data + M6502_GET_C(handle->state.sr);
  return M6502_OK;
}

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
    ADD_HANDLER(NOP);
    default:
      return handle_NOP;
  }
}
