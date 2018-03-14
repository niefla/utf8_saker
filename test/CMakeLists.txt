cmake_minimum_required (VERSION 3.1.0)
set (CMAKE_CXX_STANDARD 11)
project (utf8_saker_tests)

set (UTF8_STRING_DIR ../src)
set (SOURCES tests_rope.cpp)

add_library (main OBJECT main.cpp)

add_executable (utf8_saker_tests ${SOURCES} $<TARGET_OBJECTS:main>)
target_include_directories (utf8_saker_tests PRIVATE ${UTF8_STRING_DIR})
configure_file (dummy ${CMAKE_BINARY_DIR}/dummy COPYONLY)
