cmake_minimum_required(VERSION 3.26)
project(H1)

set(CMAKE_CXX_STANDARD 20)

add_executable(H1 main.cpp
        funclib.h
        fpm/fixed.hpp
        fpm/math.hpp
        engine.h)
