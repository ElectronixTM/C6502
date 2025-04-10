#include "m6502_stack_utils.h"
#include "m6502_chip_instance.h"
#include "m6502_byte_tweaks.h"

#define STACK_PAGE 0x01
#define TO_STACK_ADDR(base) (STACK_PAGE << 8) + (base)

void m6502_push(M6502_HANDLE handle, uint8_t data)
{
  handle->bus.write(handle->bus.handle, TO_STACK_ADDR(handle->state.sp), data);
  handle->state.sp--;
}

uint8_t m6502_pull(M6502_HANDLE handle)
{
  handle->state.sp--;
  return handle->bus.read(
      handle->bus.handle,
      TO_STACK_ADDR(handle->state.sp)
      );
}

void m6502_push_addr(M6502_HANDLE handle, uint16_t addr)
{
  uint8_t high_byte = M6502_GET_HB(addr);
  m6502_push(handle, high_byte);
  uint8_t low_byte = M6502_GET_LB(addr);
  m6502_push(handle, low_byte);
}
