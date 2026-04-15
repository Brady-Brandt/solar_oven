cmake_minimum_required(VERSION 3.13)
set(PICO_BOARD pico_w)
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
    src/wifi.c
    src/ntp.c
    src/debug.c
    src/sensors.c
    src/touchscreen.c
)

option(ENABLED_WIFI "Enable WIFI" 0)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "DEBUG Mode")
    add_compile_definitions(DEBUG=1)
endif()

if(ENABLE_WIFI)
    if(NOT DEFINED ENV{SSID} OR NOT DEFINED ENV{PASSWD})
        message(FATAL_ERROR "Environment variables not set")
    endif()
    add_compile_definitions(
        PRIVATE SSID=\"$ENV{SSID}\"
        PRIVATE PASSWD=\"$ENV{PASSWD}\"
    )
    message(STATUS "Wifi enabled")
    add_definitions(-DENABLE_WIFI)
    target_link_libraries(
        solar_oven 
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_mbedtls
        pico_mbedtls
    )
else()
    add_compile_definitions(
        PRIVATE SSID=""
        PRIVATE PASSWD=""
    )
    message(STATUS "Wifi disabled")
endif()

pico_generate_pio_header(solar_oven ${CMAKE_CURRENT_LIST_DIR}/src/pulsewidth.pio)
target_include_directories(solar_oven PRIVATE fonts/)
target_include_directories(solar_oven PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/wconfig)

pico_enable_stdio_usb(solar_oven 1)

pico_enable_stdio_uart(solar_oven 1)

pico_add_extra_outputs(solar_oven)

target_link_libraries(solar_oven 
    pico_stdlib
    hardware_spi
    hardware_rtc
    hardware_pio
    hardware_adc
    hardware_i2c
)
