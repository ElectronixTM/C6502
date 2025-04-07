#ifndef C_6502_H
#define C_6502_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Микроконтроллер общается со всей внешней аппаратурой, в том числе с
 * памятью, напрямую. Все его обращения происходят через шину. В этом
 * файле структура шины нужна лишь как интерфейс.
 */
struct m6502_Bus
{
  uint16_t address; /**< Содержимое шины адреса. используется
                      для определения целевого устроства для данных*/
  uint8_t data; /**< Содержимое шины данных */
};

/**
 * Состояния процессора взаимодейтсвия с шиной. Чтение, запись или
 * отсутствие взаимодействия
 */
enum m6502_RWState
{
  READ, /**< состояние чтения с шины данных */
  WRITE, /**< состояние записи на шину данных */
  NONE /** показывает, что микроконтроллер не завимодействует с шиной */
};

/**
 * Флаги состояния микроконтроллера. Обновляются после выполнения
 * части опкодов, например арифметических операций
 */
enum m6502_StateFlags
{
  m6502_C = (1 << 0), /**< CARRY_FLAG - обычно последний непоместившийся
                     в операцию бит */
  m6502_Z = (1 << 1), /**< ZERO_FLAG - результат последней операции дал ноль */
  m6502_I = (1 << 2), /**< INTERRUPTS - 1 - прерывания выключены */
  m6502_D = (1 << 3), /**< DECIMAL_MODE - не планируется поддержка */
  m6502_B = (1 << 4), /**< BREAK - стоит в единице, когда вызывается
                     операция BREAK */
  m6502_U = (1 << 5), /**< не используется */
  m6502_V = (1 << 6), /**< OVERFLOW - переполнение в результате
                          последней операции */
  m6502_N = (1 << 7), /**< NEGATIVE - отрицательное число в результате
                          последней операции */
};

/**
 * Структура состояния микроконтроллера. Содержит информацию о регистрах,
 * используемой шине, состоянии взаимодействия с шиной и флагах
 */
struct m6502_State
{
  uint8_t a; /**< аккумулятор, неявно задействуется для части
                  арифметических операций */
  uint8_t x;
  uint8_t y;
  uint16_t pc; /**< счетчик команд - указывает на следуюшую
                    к прочтению команду */
  uint8_t sp; /**< указатель стека */
  uint8_t sr; /**< флаги. Подробнее см. перечисление m6502_StateFlags */
  struct m6502_Bus* bus; /**< шина, к которой подключен микроконтроллер */
  m6502_RWState rw; /**< намерения процессора относительно шины -
                         чтение/запись/отстуствие взаиомдействия */
};

/**
 * Аллоцирует на heap новый экземпляр состояния процессора и возвращает
 * указатель на него. Пользователю затем следует освободить выделенное
 * под состояние место при помощи функции m6502_free_state
 *
 * @param bus шина, к которой подключено устройство
 */
struct m6502_State* m6502_create_state(struct m6502_Bus* bus);
void m6502_trigger_clock(struct m6502_State*);
// void m6502_interrupt(struct m6502_State*);
// void m6502_non_maskable_interrupt(struct m6502_State*);
void m6502_free_state(struct m6502_State*);

#endif
