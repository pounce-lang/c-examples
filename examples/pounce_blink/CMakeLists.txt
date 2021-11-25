cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)
project(pounce_blink_project)

pico_sdk_init()

add_executable(pounce_blink
	hello-blink.c
)

pico_enable_stdio_usb(pounce_blink 1)
pico_enable_stdio_uart(pounce_blink 1)

pico_add_extra_outputs(pounce_blink)

target_link_libraries(pounce_blink 
	pico_stdlib
)

