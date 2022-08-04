# DwarfMaster
12-05-2022

Midi controller for Mod Devices Dwarf

As no commercially avalible midi foot controller meet all of my requirements I have decided to build one myself.

The controller is build with a Teensy 4.1 as the "brain". The Teensy is fitted with a MicroSD card for storrage of data.
The controller has a small touch screen that shows selected bank, patch and other relevant info. The "programming" of the controller is also done with the touch screen.

This repository will hold source files and other documentation related to the build.
This is not ment to be a community project as the controller is tailored to my specific needs, but all code is avalible to anyone doing a similar project.
This is my first project using the Arduino/Teensy platform, and the code shows that I'm learning as I go - so quite a few "hacks" and inconsistencies can be found.

The code is developed using VisualTeensy and Visual Studio Code.
These external libraries are used: ArduinoJson 6.19.4, Adafruit_BusIO 1.11.5, Adafruit_TouchScreen 1.1.3, Adafruit_GFX_Library 1.10.14, MCUFRIEND_kbv 2.9.9 
These internal libraries are used: EEPROM 2.0, SD 2.0.0, SdFat 2.1.0, SPI 1.0, Wire 1.0, MIDI Library 5.0.2
