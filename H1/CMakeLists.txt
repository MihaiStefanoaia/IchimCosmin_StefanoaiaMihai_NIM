cmake_minimum_required(VERSION 3.24)
project(H1)

set(CMAKE_CXX_STANDARD 20)

add_executable(H1 main.cpp
        funclib.h
        fpm/fixed.hpp
        fpm/math.hpp
        engine.h
        genome.h
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wpedantic")
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-static")
#set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-static")
