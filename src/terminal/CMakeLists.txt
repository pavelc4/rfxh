add_library(terminal STATIC
  terminal.cpp
)

target_include_directories(terminal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
