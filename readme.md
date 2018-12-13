# Genesis Converter

This is code for (probably) yet another Sega Genesis converter. This code
is intended for use with a Teensy 2.0, and is intended to intelligently
handle a 3-button pad, a 6-button pad, or a 1/2 button SMS or 8-bit computer
pad/joystick. Each configuration will use the available buttons as 
the primary converted buttons, and won't end up with phantom button
presses from incompatible pads (unlike some commercial converters)

There are three branches of development maintained in this repository,
depdending on intended use for the converter:

 * **generic_usb** : Converts the pad to a generic USB HID controller. 
    Defines two axis and up to 10 buttons, though only a max of 8 are used.
    The extra gap is to place the start/mode buttons on buttons 9 and 10
    in order for better default compatibility with some software.
    
    This branch is usually merged into **master** as well.
    
 * **ps3pad** : Although the above branch will usually work with most
    PS3 software, this branch is intended to appear more like a native
    PS3 arcade stick. Several PS3 to other console converters will 
    actually recognize and work with this mode. Use on a PC is less
    ideal, because the dpad is mapped to a dpad, and the X/Y gamepad
    axis does not move.

 * **keyboard** : Directly outputs keyboard keys. No joystick to keyboard
    software required! Default keys are **Z**, **X**, **C**, then 
    **A**, **S** and **D**. Start is **Enter**, while Mode is **Space**.


## Wiring

All versions of the converter expect the same wiring, though you can 
modify your copy of *genesis_pad.c* if alternate wiring is needed.
By default, all data wires are expected to be connected to Port B on the
Teensy. Wiring is as follows:

Gamepad Port Pin (DE9 connector) | Teensy Port | Use
-------------------------------- | ----------- | ---
1 | B3 | Up
2 | B2 | Down
3 | B1 | Left
4 | B0 | Right
5 | VCC | + 5V
6 | B6 | A/B button
7 | B5 | MUX select line
8 | GND | GND
9 | B4 | C/start button

## Dependencies

Build dependencies are the same as for the Teensy C examples. See
the [PJRC getting started page](https://www.pjrc.com/teensy/gcc.html) 
for more details. Debian-derivative users can just install from repository:

    # apt install gcc-avr binutils-avr avr-libc

The `make install` target assumes use of the `teensy_loader_cli` 
application, and that it is installed into the system path. See
the [PJRC Teensy Loader Page](https://www.pjrc.com/teensy/loader_cli.html)
for more details. The GUI loader will obviously work too, but that's
boring.

Since the Teensy loader makefile has no install line, I suggest simply
copying to /usr/local/bin after compiling.

## Build

Building is pretty standard make stuff. To compile:

    $ make

To load to the Teensy, assuming Teensy Loader is on the path,
press the reset button on the Teensy then run:

    $ make install
