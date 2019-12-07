cmake_minimum_required(VERSION 3.15)
project(matamazom C)

set(CMAKE_C_STANDARD 99)

add_executable(matamazom matamazom.c list.h test_utilities.h matamazom.h matamazom_main.c amount_set.c
        amount_set.h matamazom_tests.c matamazom_tests.h
        matamazom_print.c matamazom_print.h )
target_link_libraries(matamazom ${CMAKE_SOURCE_DIR}/libmtm/win32/libmtm.a)