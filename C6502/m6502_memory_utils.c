#include "m6502_memory_utils.h"
#include "m6502_chip_instance.h"
#include "m6502_byte_tweaks.h"


uint8_t read_byte(M6502_HANDLE handle, uint16_t addr)
{
  return handle->bus.read(handle->bus.handle, addr);
}

void write_byte(M6502_HANDLE handle, uint16_t addr, uint8_t data)
{
  handle->bus.write(handle->bus.handle, addr, data);
}

uint16_t read_word(M6502_HANDLE handle, uint16_t addr)
{
  uint8_t lb = handle->bus.read(handle->bus.handle, addr);
  uint8_t hb = handle->bus.read(handle->bus.handle, addr+1);
  return M6502_PACK_2_BYTES(lb, hb);
}

void write_word(M6502_HANDLE handle, uint16_t addr, uint16_t data)
{
  handle->bus.write(handle->bus.handle, addr, M6502_GET_LB(data));
  handle->bus.write(handle->bus.handle, addr+1, M6502_GET_HB(data));
}
