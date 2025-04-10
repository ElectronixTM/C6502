cmake_minimum_required(VERSION 3.10)

project(
  C6502
  LANGUAGES C
  VERSION 0.0.1
  DESCRIPTION "Software emulation of old MOS Technonlogy 6502 8-bit processor"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

add_executable(
  test ""
)

target_include_directories(
  test PRIVATE
  include
)

target_sources(
  test PRIVATE
  C6502/main.c
)

