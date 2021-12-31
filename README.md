Cross Chip Blinky
=================

My experiments with various AVR chips I have with writing fairly bare bones blink programs.

Inspiraion for the millis code is taken from (MighyCore)[https://github.com/MCUdude/MightyCore]

This code should work on nearly any AVR, but I've only setup the code to work with (so far):-

ATtiny13
ATtiny85
ATmega32a
ATmega328p

The makefiles are setup for using a Raspberry PI which happens to permanently have a breadboard attached.
I use GPIO 21 to control the reset pin, and the standard SCLK, MISO and MOSI pins.

Soon will come ATmega1284 (I doubt that will need much work) and the newish ATmega4809, for which I'll
have to figure out the hardware and software to do UPDI programming.
