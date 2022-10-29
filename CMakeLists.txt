cmake_minimum_required(VERSION 3.10.0)
project(EnumHelper VERSION 0.1.0)

add_definitions("-fconstexpr-depth=200")

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic-errors")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_BUILD_TYPE "Debug")

add_executable(enum_iterator examples/EnumIterator.cpp)
target_include_directories(enum_iterator PRIVATE include)
set_property(TARGET enum_iterator PROPERTY CXX_STANDARD 11)

add_executable(enum_iterator_17 examples/EnumIterator.cpp)
target_include_directories(enum_iterator_17 PRIVATE include)
set_property(TARGET enum_iterator_17 PROPERTY CXX_STANDARD 17)

add_executable(enum_iterator_20 examples/EnumIterator.cpp)
target_include_directories(enum_iterator_20 PRIVATE include)
set_property(TARGET enum_iterator_20 PROPERTY CXX_STANDARD 20)

add_executable(enum_iterator_macro examples/EnumIteratorWithoutMacroMagic.cpp)
target_include_directories(enum_iterator_macro PRIVATE include)
set_property(TARGET enum_iterator_macro PROPERTY CXX_STANDARD 11)

add_executable(enum_iterator_cout examples/EnumIteratorCout.cpp)
target_include_directories(enum_iterator_cout PRIVATE include)
set_property(TARGET enum_iterator_cout PROPERTY CXX_STANDARD 20)
