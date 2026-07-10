cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_COMPILER "clang++")

project(rfxh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(rfxh
    src/main.cpp
    src/terminal/terminal.cpp
    src/text/utf8.cpp
    src/text/shading.cpp
    src/text/charweight.cpp
    src/config/defaults.cpp
    src/config/cli.cpp
    src/config/loader.cpp
    src/logo/loader.cpp
    src/logo/processor.cpp
    src/logo/detect.cpp
    src/logo/colors.cpp
)

target_include_directories(rfxh PRIVATE src)
