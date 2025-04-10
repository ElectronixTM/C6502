#include <malloc.h>
#include <C6502.h>
#include "m6502_chip_instance.h"
#include "m6502_opcode_handlers.h"
#include "m6502_stack_utils.h"
#include "m6502_byte_tweaks.h"

M6502_HANDLE m6502_get_handle(M6502_BUS_ATTACH bus_attach,
                              M6502_BUS_DETACH bus_detach,
                              M6502_BUS_READ bus_read,
                              M6502_BUS_WRITE bus_write)
{
  M6502_HANDLE bus_handle = bus_attach();
  struct m6502_ChipInstance* m6502_handle = malloc(sizeof(struct m6502_ChipInstance));
  m6502_handle->bus.handle = bus_handle;
  m6502_handle->bus.detach = bus_detach;
  m6502_handle->bus.read = bus_read;
  m6502_handle->bus.write = bus_write;

  // обнуление значений все регистров
  struct m6502_State null_state = {0,};
  m6502_handle->state = null_state;
  m6502_handle->state.rw = M6502_RW_NONE;
  return m6502_handle;
}

void m6502_free_handle(M6502_HANDLE handle)
{
  handle->bus.detach(handle);
  free(handle);
}

void m6502_trigger_clock(M6502_HANDLE handle)
{
  if (handle->cycles_remaining > 0)
  {
    handle->cycles_remaining--;
    return;
  }

  uint8_t opcode = handle->bus.read(
      handle->bus.handle,
      handle->state.pc
      );

  const struct m6502_OpCodeDesc* desc = m6502_fetch_opcode_desc(opcode);
  // задаем предварительное необходимое число тактов на инструкцию.
  // В дальнейшем это число может увеличиться
  handle->cycles_remaining = desc->minrequiredcycles;
  OPCODE_HANDLER opcode_handler = m6502_get_opcode_handler(desc->mnemonic);
  opcode_handler(handle, desc);
}

#define M6502_IRQ_LB 0xFFFA
#define M6502_IRQ_HB 0xFFFB



void m6502_trigger_irq(M6502_HANDLE handle)
{
  // Если прерывания отключены
  if (handle->state.sr & M6502_I)
  {
    return;
  }
  // PC
  m6502_push_addr(handle, handle->state.pc);
  // SR
  m6502_push(handle, handle->state.sr);

  // читаем адрес обработчика прерывания
  uint8_t irq_handler_lb = handle->bus.read(handle->bus.handle, M6502_IRQ_LB);
  uint8_t irq_handler_hb = handle->bus.read(handle->bus.handle, M6502_IRQ_HB);
  // записываем прочитанный адрес в pc
  handle->state.pc = M6502_PACK_2_BYTES(irq_handler_lb, irq_handler_hb);
}
