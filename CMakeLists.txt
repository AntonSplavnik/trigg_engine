cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(TriggEngine C CXX ASM)

# C++11 standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

pico_sdk_init()

# Main executable
add_executable(${PROJECT_NAME}
    main.cpp
    drivers/spi.cpp
    drivers/display.cpp
    drivers/buttons.cpp
    drivers/st7735_driver.cpp
    engine/graphics/framebuffer.cpp
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/engine/graphics
)

# Link Pico libraries
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_spi
    hardware_gpio
    hardware_dma
    hardware_pwm
)

# Enable USB output (for printf debugging)
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Generate UF2 file:
pico_add_extra_outputs(${PROJECT_NAME})

# Compiler warnings
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -O2
)
