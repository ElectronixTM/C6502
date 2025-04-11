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

int handle_NOP(M6502_HANDLE handle, const struct m6502_OpCodeDesc* desc)
{
  SKIP_UNTIL_ZERO(handle->state.pc);
  handle->state.pc += desc->instrsize;
  handle->cycles_remaining = desc->minrequiredcycles;
  return 0;
}

int handle_ADC(M6502_HANDLE handle, const struct m6502_OpCodeDesc* desc)
{
  SKIP_UNTIL_ZERO(handle->state.pc);
  ADDRESSING_HANDLER addressing_handler = m6502_get_addressing_handler(desc->addressmode);
  uint8_t extra_cycles;
  struct m6502_Operands operands = addressing_handler(
      handle,
      get_raw_operands(handle, desc->instrsize).word,
      &extra_cycles
      );
  uint8_t data;
  uint16_t real_addr;
  switch(operands.type)
  {
    case RESULT_DATA:
      data = operands.content.data;
      break;
    case RESULT_ADDR:
      data = handle->bus.read(handle->bus.handle, operands.content.address);
      break;
    case RESULT_IND:
      real_addr = read_word(handle, operands.content.address);
      data = read_byte(handle, real_addr);
      break;
    case RESULT_IMPL:
      return M6502_ERR;
  }
  return M6502_OK;
}

OPCODE_HANDLER m6502_get_opcode_handler(enum m6502_Mnemonic mnemonic)
{
  // Сейчас я использую максимально простую реализацию
  switch(mnemonic)
  {
    case MNEM_ADC:
      return handle_ADC;
      break;
    default:
      return handle_NOP;
  }
}
