#include <malloc.h>
#include <C6502.h>
#include "m6502_chip_instance.h"

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

