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
    src/gather/gather.cpp
    src/gather/system/os.cpp
    src/gather/system/host.cpp
    src/gather/system/kernel.cpp
    src/gather/system/uptime.cpp
    src/gather/system/packages.cpp
    src/gather/system/shell.cpp
    src/gather/system/locale.cpp
    src/gather/hardware/display.cpp
    src/gather/hardware/cpu.cpp
    src/gather/hardware/gpu.cpp
    src/gather/hardware/memory.cpp
    src/gather/hardware/disk.cpp
    src/gather/hardware/battery.cpp
    src/gather/desktop/wm.cpp
    src/gather/desktop/terminal_info.cpp
    src/gather/desktop/net.cpp
    src/gather/desktop/gtk.cpp
    src/render/pointcloud.cpp
    src/render/rasterizer.cpp
    src/render/output.cpp
)

target_include_directories(rfxh PRIVATE src)
target_link_libraries(rfxh PRIVATE m)
