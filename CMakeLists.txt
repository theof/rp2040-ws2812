cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(leds)
pico_sdk_init()
include(lib/Pico-DMX/interfaceLibForPicoSDK.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(WARN_CFLAGS -Wall -Wextra)

add_executable(firmware_array
	main_array.cpp
	draw.cpp
)

add_executable(firmware_dmx2pixel
	main_dmx2pixel.cpp
)


target_compile_options(firmware_array PRIVATE ${WARN_CFLAGS})
target_include_directories(firmware_array PUBLIC .)
target_link_libraries(firmware_array pico_stdlib hardware_pio hardware_dma picodmx)
pico_add_extra_outputs(firmware_array)

target_compile_options(firmware_dmx2pixel PRIVATE ${WARN_CFLAGS})
target_include_directories(firmware_dmx2pixel PUBLIC .)
target_link_libraries(firmware_dmx2pixel pico_stdlib hardware_pio hardware_dma picodmx)
pico_add_extra_outputs(firmware_dmx2pixel)
