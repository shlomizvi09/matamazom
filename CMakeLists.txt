cmake_minimum_required(VERSION 3.15)
project(matamazom C)

set(CMAKE_C_STANDARD 99)

add_executable(matamazom matamazom.c main.c list.h amount_set.c amount_set.h matamazom.h matamazom_print.c matamazom_print.h set.h)
target_link_libraries(matamazom ${CMAKE_SOURCE_DIR}/libmtm/win32/libmtm.a)