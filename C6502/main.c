#include <stdio.h>
#include <C6502.h>

/**
 * Это тестовый файлик для проверки функциональности до того,
 * как закончено написание всео остального
 */

// тестовая функция main
int main()
{
  printf("test execution");
  M6502_HANDLE handle = m6502_get_handle(NULL, NULL, NULL, NULL);
  m6502_free_handle(handle);
  return 0;
}
