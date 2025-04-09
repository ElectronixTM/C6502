#ifndef M6502_OPCODES_H
#define M6502_OPCODES_H

#include <stdint.h>
#include "m6502_mnemonics.h"

/**
 * Все возможные режимы адрессации микроконтроллера MOS 6502
 */
enum m6502_AddressingModes
{
  AM_A,
  AM_ABS,
  AM_ABS_X,
  AM_ABS_Y,
  AM_IMM,
  AM_IMPL,
  AM_IND,
  AM_X_IND,
  AM_IND_Y,
  AM_REL,
  AM_ZPG,
  AM_ZPG_X,
  AM_ZPG_Y
};

/**
 * В описании опкодов хранится информация о номере опкода,
 * адрессации операндов, длине команды целиком, минимальное
 * количество циклов, необходимых для исполнения
 */
struct m6502_OpCodeDesc
{
  enum m6502_Mnemonic mnemonic; /**< код операции из списка известрных эмулятору.
                                    Остальные опкоды не эмулируются и фактически
                                    становятся NOP'ами */
  uint8_t instrsize; /**< размер инструкции вместе с опкодом и операндами */
  uint8_t minrequiredcycles; /**< Минимальное количество сигналов clock,
                                 необходимых для выполнения инструкции 
                                 некоторые виды адрессации добавляют такты */
  enum m6502_AddressingModes addressmode; /**< режим адресации инструкции */
};

/**
 * Функция, проверяющая статическую таблицу опкодов и возвращающая указатель на
 * его описание. Пользователю не требуется освобождать память по этому указателю
 */
const struct m6502_OpCodeDesc* m6502_fetch_opcode_desc(uint8_t opcode);

#endif
