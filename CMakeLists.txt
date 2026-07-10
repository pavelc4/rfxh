cmake_minimum_required(VERSION 3.20)
project(rfxh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(rfxh src/main.cpp)
target_link_libraries(rfxh PRIVATE text)
