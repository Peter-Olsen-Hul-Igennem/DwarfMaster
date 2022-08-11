# DwarfMaster
11-08-2022

Midi controller for Mod Devices Dwarf

As no commercially avalible midi foot controller meet all of my requirements I have decided to build one myself.

The controller is build with a Teensy 4.1 as the "brain". The Teensy is fitted with a MicroSD card for storrage of data.
The controller has a small touch screen that shows selected bank, patch and other relevant info. The "programming" of the controller is also done with the touch screen.

This repository will hold source files and other documentation related to the build.
This is not ment to be a community project as the controller is tailored to my specific needs, but all code is avalible to anyone doing a similar project.
This is my first project using the Arduino/Teensy platform, and the code shows that I'm learning as I go - so quite a few "hacks" and inconsistencies can be found.

The code is developed using VisualTeensy and Visual Studio Code.
These libraries are used: ArduinoJson 6.19.4, Adafruit_BusIO 1.11.5, Adafruit_TouchScreen 1.1.3, Adafruit_GFX_Library 1.10.14, MCUFRIEND_kbv 2.9.9, EEPROM 2.0, SD 2.0.0, SdFat 2.1.0, SPI 1.0, Wire 1.0, MIDI Library 5.0.2

The controller has 6 footswitches that can be set up as a "preset" of "toggle" button. The preset footswitches are mutually exclusive. Pressing a footswitch can trigger up to 16 PC and 16 CC messages being send. Preset footswitches can be configured to have a secondary press function that can send another 16 PC and 16 CC massages.

The six footswitches constitute a Bank. There are 100 banks. A PC message can be send on changing to a new bank (to load a pedalboard in the Dwarf). Bank up and down is done with pressing footswitch 1+2 (down) and 2+3 (up).

All configuration of footswitches and banks (names, pc's, cc's, preset/toggle, 2. function) is done via the touchscreen with footswitch 3 as "ok" button and footswitch 1 as "cancel/exit".
There is utility functions for copying banks and footswitches as well as sending a single CC (for midi-learn purpose).

All midi messages is send on both the USB port as well as a standard 5 pin midi connector.
