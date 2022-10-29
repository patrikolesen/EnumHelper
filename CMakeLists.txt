cmake_minimum_required(VERSION 3.10.0)
project(EnumHelper VERSION 0.1.0)

add_definitions("-fconstexpr-depth=200")

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic-errors")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_BUILD_TYPE "Debug")

add_executable(EnumIterator examples/EnumIterator.cpp)
target_include_directories(EnumIterator PRIVATE include)
set_property(TARGET EnumIterator PROPERTY CXX_STANDARD 11)


