cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_COMPILER "clang++")

project(rfxh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(rfxh
    src/main.cpp
    src/terminal/terminal.cpp
)

target_include_directories(rfxh PRIVATE src)
