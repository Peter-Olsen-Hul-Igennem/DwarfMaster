# DwarfMaster
12-05-2022

Midi controller for Mod Devices Dwarf

As no commercially avalible midi foot controller meet all of my requirements I have decided to build one myself.

The controller is build with a Teensy 4.1 as the "brains". The Teensy is fitted with a MicroSD card for storrage of data.
The controller has a small touch screen that shows selected bank and patch and other relevant info. The "programming" of the controller is also done with the touch screen.

This repository will hold source files and other documentation related to the build.
This is not ment to be a community project as the contoller is tailored to my specific needs, but all code is avalible to use by anyone doing a similar project.

These external libraries are used: ArduinoJson 6.19.4, Adafruit_BusIO 1.11.5, Adafruit_TouchScreen 1.1.3, Adafruit_GFX_Library 1.10.14, MCUFRIEND_kbv 2.9.9 
These internal libraries are used: EEPROM 2.0, SD 2.0.0, SdFat 2.1.0, SPI 1.0, Wire 1.0
