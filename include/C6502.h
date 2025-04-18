#ifndef C_6502_H
#define C_6502_H

#include <stdint.h>
#include <stdbool.h>

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
  uint8_t sp; /**< STACK_POINTER - указатель стека */
  uint8_t sr; /**< STATE_REGISTER - флаги. Подробнее см.
                   перечисление m6502_StateFlags */
  struct m6502_Bus* bus; /**< шина, к которой подключен микроконтроллер */
  enum m6502_RWState rw; /**< намерения процессора относительно шины -
                         чтение/запись/отстуствие взаиомдействия */
};

/**
 * Указатель на структуру для работы с экземпляром микроконтроллера.
 * Получение экземпляра микроконтроллера потребует затем его явного 
 * освобождения при помощи предоставляемых функций
 */
typedef struct m6502_ChipInstance* M6502_HANDLE;

/**
 * Указатель на какую-то информацию о шине. Для эмулятора не имеет значения,
 * что именно это за информация, и этот интерфейс нужен только для увеличения гибкости.
 * В действительности в своих обращениях к шине
 * эмулятор будет всегда первым параметром передавать то, что ему будет
 * возвращено из функции подключения к шине
 */
typedef void* M6502_BUS_HANDLE;

/**
 * Функция, присоединяющая устройство к шине. возвращенный указатель будет
 * всегда передаваться в качестве первого параметра во всех обращениях к шине
 */
typedef M6502_BUS_HANDLE (*M6502_BUS_ATTACH)();

/**
 * Функция чтения данных с шины. Должна принимать 16 битный адрес (с точки
 * зрения схемотехники - и содержимое шины адреса) и отдавать
 * данные по этому адресу. Данные всегда представляются 8-битным числом
 */
typedef uint8_t (*M6502_BUS_READ)(M6502_BUS_HANDLE, uint16_t address);

/**
 * Функция записи данных на шину. Должна принимать 16 битный адрес
 * (содержимое шины адреса в схемотехнике) и 8 битные данные (содержимое
 * шины данных в схемотехнике)
 */
typedef void (*M6502_BUS_WRITE)(M6502_BUS_HANDLE, uint16_t address, uint8_t data);

/**
 * Функция открепления микроконтроллера от шины. будет вызвана при
 * уничтожении экземпляра микроконтроллера
 */
typedef void (*M6502_BUS_DETACH)(M6502_BUS_HANDLE);
/**
 * Аллоцирует на heap новый экземпляр процессора и возвращает
 * указатель на него. Пользователю затем следует освободить выделенное
 * под экземпляр место при помощи функции m6502_free_handle
 *
 * Принимает функции присоединения к шине, отсоединения от шины,
 * чтения с шины и записи на шину
 *
 * Первая вызовется при создании, всем остальным помимо прочего
 * первым параметром будет передаваться возвращаемое значение первой
 */
M6502_HANDLE m6502_get_handle(M6502_BUS_ATTACH, M6502_BUS_DETACH,
                              M6502_BUS_READ, M6502_BUS_WRITE);

/**
 * Принимает экземпляр микроконтроллера, отключает его от шины, 
 * вызывая функцию M6502_BUS_DETACH, переданную при создании.
 */
void m6502_free_handle(M6502_HANDLE);

/**
 * Функция, иммитирующая подачу сигнала clock на микроконтроллер.
 * Обычно функция вызывается каждый такт частотного генератора
 */
void m6502_trigger_clock(M6502_HANDLE);

/**
 * Функция вызова прерывания. Заставляет процессор
 * перескочить по адресам обработки прерывания, которые
 * можно посмотреть в документации микроконтроллера
 */
void m6502_trigger_irq(M6502_HANDLE);

/**
 * Функция вызова немаскируемого прерывания. Такие прерывания
 * игнорируют выставленные биты в процессоре и нужны в крайних
 * случаях
 */
void m6502_trigger_nmi(M6502_HANDLE);

#endif
