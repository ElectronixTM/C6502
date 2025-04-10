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
  handle->bus.detach(handle->bus.handle);
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

/**
 * Функция кладет на стек текущий pc и sr
 */
void _prepare_to_interrupt(M6502_HANDLE handle)
{
  // push PC
  m6502_push_addr(handle, handle->state.pc);
  // push SR
  m6502_push(handle, handle->state.sr);
}

/**
 * Функция чтения адреса из памяти в формате little endian
 */
uint16_t _read_addr_from_bus(M6502_HANDLE handle, uint16_t addr)
{

  uint8_t lb = handle->bus.read(handle->bus.handle, addr);
  uint8_t hb = handle->bus.read(handle->bus.handle, addr+1);
  return M6502_PACK_2_BYTES(lb, hb);
}

#define M6502_IRQ_HANDLER_ADDR 0xFFFA

void m6502_trigger_irq(M6502_HANDLE handle)
{
  // Если еще не закончили с предыдущей командой
  if (handle->cycles_remaining > 0)
  {
    return;
  }
  // Если прерывания отключены
  if (handle->state.sr & M6502_I)
  {
    return;
  }
  _prepare_to_interrupt(handle);

  // читаем адрес обработчика прерывания
  // записываем прочитанный адрес в pc
  handle->state.pc = _read_addr_from_bus(handle, M6502_IRQ_HANDLER_ADDR);
}

#define M6502_NMI_HANDLER_ADDR 0xFFFE

void m6502_trigger_nmi(M6502_HANDLE handle)
{
  if (handle->cycles_remaining > 0)
  {
    return;
  }
  _prepare_to_interrupt(handle);
  handle->state.pc = _read_addr_from_bus(handle, M6502_NMI_HANDLER_ADDR);
}

// количество тактов, которое консоль тратит
// на то, чтобы запуститься и записать в pc
// нужный адрес
#define M6502_INIT_CYCLES_AMOUNT 7

// адрес, по которому перейдет pc, когда
// выполнится инициализация микроконтроллера
#define M6502_RESET_HANDLER_ADDR 0xFFFC

void m6502_trigger_reset(M6502_HANDLE handle)
{
  handle->cycles_remaining = 7;
  struct m6502_State null_state = {0,};
  handle->state = null_state;
  handle->state.pc = _read_addr_from_bus(handle, M6502_RESET_HANDLER_ADDR);
}
