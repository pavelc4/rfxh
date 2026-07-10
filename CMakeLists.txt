cmake_minimum_required(VERSION 3.20)

if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_CXX_COMPILER "clang++")
endif()

project(rfxh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE MATCHES Release AND NOT CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

# Platform detection
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(RFXH_PLATFORM win32)
    add_compile_definitions(_WIN32)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(RFXH_PLATFORM posix)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(RFXH_PLATFORM posix)
    add_compile_definitions(__ANDROID__)
else()
    set(RFXH_PLATFORM posix)
endif()

# Platform-specific sources
set(PLATFORM_SOURCES
    src/platform/posix/terminal.cpp
    src/platform/posix/process.cpp
    src/platform/posix/sysinfo.cpp
)

if(RFXH_PLATFORM STREQUAL "win32")
    set(PLATFORM_SOURCES
        src/platform/win32/terminal.cpp
        src/platform/win32/process.cpp
        src/platform/win32/sysinfo.cpp
    )
endif()

# Core sources (always compiled)
set(CORE_SOURCES
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
    src/render/pointcloud.cpp
    src/render/rasterizer.cpp
    src/render/output.cpp
    src/engine/app.cpp
)

# POSIX-only gatherer sources (Linux/macOS)
set(POSIX_GATHER_SOURCES
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
)

add_executable(rfxh
    ${CORE_SOURCES}
    ${PLATFORM_SOURCES}
)

if(RFXH_PLATFORM STREQUAL "win32")
    target_sources(rfxh PRIVATE src/gather/gather_win32.cpp)
else()
    target_sources(rfxh PRIVATE ${POSIX_GATHER_SOURCES})
endif()

target_include_directories(rfxh PRIVATE src)
target_compile_options(rfxh PRIVATE -Wno-trigraphs)

if(NOT WIN32)
    target_link_libraries(rfxh PRIVATE m)
endif()
