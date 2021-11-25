# c-examples
a few projects that use pounce-lang/c-core for as an embedded microprocessor language

### examples/pounce-blink
Blink, the "hello world" of microprocessor code examples is written in Pounce. In three variation: one using sleep; a second that polls for elapsed time; the third using microprocessor timers, 

### examples/flicker-sensor
RP2040 pico project samples light level at 50 KHz and runs an fft to obtain frequency data on a flickering light source. The fft is in "C" code and pounce code act on the frequency domain to report out any significant frequency on a 7-segment numeric display.  




