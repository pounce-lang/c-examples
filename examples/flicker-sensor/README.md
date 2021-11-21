# flicker-sensor
A light "flicker" meter. Measures the frequency of a flickering light source. Target hardware is the RP2040 (a $4 board) with a light sensor and two seven-segment displays, a few other leds and current limiting resistors. Could be converted to a sound frequency sensor (aka Guitar Tuner) as well :). 

## hardware
 * RP2040 pico board (lots of GPIO and two M0-cores)
 * (2) 7-segment displays
 * light sensor 
 * resistor (voltage divider for the light sensor)
 * other leds (used to indicate Hz, KHz, MHz)

## software
* RP2040 c-sdk using
    * adc
    * dma
    * multi-core
* Fourier transform in "C" (code from ...) on M0-core-0
* Pounce-lang (for post fft calculations and display) on M0-core-1

## build a uf2 file for the rp2040
### install gcc-arm compiler (for your specific hardware) from ~/bin
tar xvf ../gcc-arm-none-eabi-<your specific hardware>-linux.tar.bz2

### set the path
export PATH=/home/$USER/bin/gcc-arm-none-eabi-<your specific>/bin:$PATH

### also requires cmake, PICO_SDK, then
mkdir build-rp2040

cd build-rp2040

cmake ..

make 

