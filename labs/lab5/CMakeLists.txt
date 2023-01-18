cmake_minimum_required(VERSION 3.16)
project(lab5)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS -pthread)

include_directories(.)

add_executable(lab5
        main.cpp)

target_link_libraries(${PROJECT_NAME} pthread)
