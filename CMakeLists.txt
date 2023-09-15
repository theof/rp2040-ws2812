cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(leds)
pico_sdk_init()
include(lib/Pico-DMX/interfaceLibForPicoSDK.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(WARN_CFLAGS -Wall -Wextra)

add_executable(firmware
	main.cpp
	draw.cpp
)
target_compile_options(firmware PRIVATE ${WARN_CFLAGS})
target_include_directories(firmware PUBLIC .)
target_link_libraries(firmware pico_stdlib hardware_pio hardware_dma picodmx)
pico_add_extra_outputs(firmware)
