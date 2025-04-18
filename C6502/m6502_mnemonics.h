#ifndef M6502_MNEMONICS_H
#define M6502_MNEMONICS_H

/**
 * Этот файл генерируется при помощи скриптов в папке scripts
 */

/**
 * Перечисление всех мнемоник ассемблера. Многие опкоды отличаются только
 * типом адрессации, но ведут себя сходно. Поэтому выделяются мнемоники,
 * описывающие основное поведение
 */
enum m6502_Mnemonic
{
  MNEM_ADC,
  MNEM_AND,
  MNEM_ASL,
  MNEM_BCC,
  MNEM_BCS,
  MNEM_BEQ,
  MNEM_BIT,
  MNEM_BMI,
  MNEM_BNE,
  MNEM_BPL,
  MNEM_BRK,
  MNEM_BVC,
  MNEM_BVS,
  MNEM_CLC,
  MNEM_CLD,
  MNEM_CLI,
  MNEM_CLV,
  MNEM_CMP,
  MNEM_CPX,
  MNEM_CPY,
  MNEM_DEC,
  MNEM_DEX,
  MNEM_DEY,
  MNEM_EOR,
  MNEM_INC,
  MNEM_INX,
  MNEM_INY,
  MNEM_JMP,
  MNEM_JSR,
  MNEM_LDA,
  MNEM_LDX,
  MNEM_LDY,
  MNEM_LSR,
  MNEM_NOP,
  MNEM_ORA,
  MNEM_PHA,
  MNEM_PHP,
  MNEM_PLA,
  MNEM_PLP,
  MNEM_ROL,
  MNEM_ROR,
  MNEM_RTI,
  MNEM_RTS,
  MNEM_SBC,
  MNEM_SEC,
  MNEM_SED,
  MNEM_SEI,
  MNEM_STA,
  MNEM_STX,
  MNEM_STY,
  MNEM_TAX,
  MNEM_TAY,
  MNEM_TSX,
  MNEM_TXA,
  MNEM_TXS,
  MNEM_TYA
};

#endif
