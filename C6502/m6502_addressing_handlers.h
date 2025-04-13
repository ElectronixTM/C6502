#ifndef M6502_ADDRESSING_HANDLERS_H
#define M6502_ADDRESSING_HANDLERS_H

#include <stdint.h>
#include <C6502.h>
#include "m6502_opcodes.h"


/**
 * Тип возвращаемого из обработчика адрессации значения
 */
enum m6502_AddressingRes_type
{
  RESULT_DATA, /**< 8-битные данные в качестве опернада */
  RESULT_ADDR, /**< 16-битный адрес в памяти, где лежат данные */
  RESULT_IND, /**< Операнд лежит по адресу, тоже расположенному в памяти.
                   Потребуется 2 обращения к шине */
  RESULT_A, /**< Место назначения - аккумулятор */
  RESULT_IMPL /**< можно игнорировать вывод функции */
};

/**
 * Структура данных для сохранения данных о построении адреса
 * или выборке операндов для инструкции. Может хранить как
 * непосредственный операнд, так и адресс, по которому тот
 * расположен на шине
 */
struct m6502_Operands{
  enum m6502_AddressingRes_type type;
  union{
    uint16_t address;
    uint8_t data;
  } content;
};

/**
 * Функция, вычисляющая операнды для режима адрессации
 * и возвращающая их в специальной структуре. В структуре
 * может содержаться адресс, по которому располагаются
 * интересующие пользователя данные. Функция не должна
 * обращаться к шине
 *
 * @param data операнды, следующие сразу за опкодом. Может быть как одним
 *             16-битным словом, так и двумя упакованными байтами
 * @param extra_cycles выходной параметр, в который записывается, сколько
 *                     дополнительных циклов сверх стандартного числа
 *                     потребовало это обращение к памяти. Дополнительные
 *                     такты нужны когда, например, при адрессации адрес
 *                     входит за пределы изначальной страницы памяти
 */
typedef struct m6502_Operands(*ADDRESSING_HANDLER)(M6502_HANDLE,
                                                   uint16_t data,
                                                   uint8_t* extra_cycles
                                                   );


/**
 * Функция, принимающая опкод и возвращающая обработчик для соответствующего
 * режима адрессации
 */
ADDRESSING_HANDLER m6502_get_addressing_handler(enum m6502_AddressingModes);

#endif
