Cross Chip Blinky
=================

My experiments with various AVR chips I have with writing fairly bare bones blink programs.

[Article on my website with some pictures](https://www.atkinson.kiwi/article/blinky_from_scratch)

Inspiraion for the millis code is taken from [MighyCore](https://github.com/MCUdude/MightyCore)

This code should work on nearly any AVR, but I've only setup the code to work with (so far):-

ATtiny13

ATtiny84 - intresting pin layout compared to the rest!

ATtiny85

ATmega32a

ATmega328p

ATmega4809 - only fancy blink for now with separate source file. It's quite different to the rest.

The makefiles are setup for using a Raspberry PI which happens to permanently have a breadboard attached.
I use GPIO 21 to control the reset pin, and the standard SCLK, MISO and MOSI pins.

Soon will come ATmega1284 (I doubt that will need much work)
