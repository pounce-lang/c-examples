cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)
project(flicker_sensor_project)

pico_sdk_init()

add_executable(flicker_sensor
	flicker-sensor.c
)

add_library(kiss_fftr awulff-pico-playground/adc_fft/kiss_fftr.c)
add_library(kiss_fft awulff-pico-playground/adc_fft/kiss_fft.c)


pico_enable_stdio_usb(flicker_sensor 1)
pico_enable_stdio_uart(flicker_sensor 1)

pico_add_extra_outputs(flicker_sensor)

target_link_libraries(flicker_sensor 
	pico_multicore
	pico_stdlib
	pico_util
	hardware_adc
	hardware_dma
	kiss_fftr
	kiss_fft
)

