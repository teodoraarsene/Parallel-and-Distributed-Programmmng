cmake_minimum_required(VERSION 3.16)
project(lab3)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -pthread)

include_directories(.)

add_executable(lab3
        main.cpp
        Matrix.cpp
        Matrix.hpp
        MatrixMultiplication.cpp
        MatrixMultiplication.hpp
        ThreadPool.cpp
        ThreadPool.hpp)
