cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(solar_oven C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_compile_options(
    -Wall
    -Wextra
    -Wshadow
)

add_executable(solar_oven
    src/main.c
    src/display.c
    src/ui.c
)

target_include_directories(solar_oven PRIVATE fonts/)

pico_enable_stdio_usb(solar_oven 1)

pico_enable_stdio_uart(solar_oven 1)

pico_add_extra_outputs(solar_oven)

target_link_libraries(solar_oven pico_stdlib hardware_spi)
