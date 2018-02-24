cmake_minimum_required (VERSION 3.1.0)
set (CMAKE_CXX_STANDARD 11)
project (utf8_string_test)

set (UTF8_STRING_DIR ../src)
set (SOURCES main.cpp ${UTF8_STRING_DIR}/utf8.hpp)

add_executable (utf8_string_test ${SOURCES})
target_include_directories (utf8_string_test PRIVATE ${UTF8_STRING_DIR})
configure_file (dummy ${CMAKE_BINARY_DIR}/dummy COPYONLY)
