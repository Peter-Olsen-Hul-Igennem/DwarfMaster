#ifndef DWARFMASTER_GLOBAL_CONST_H
#define DWARFMASTER_GLOBAL_CONST_H

#include <arduino.h>

static const uint16_t DISPLAY_WIDTH = 320;
static const uint16_t DISPLAY_HEIGHT = 240;
static const uint16_t BACKGROUND_COLOR = 0xC618; //TFT_LIGHTGREY;
static const uint16_t LINE_COLOR = 0x7BEF; //TFT_DARKGREY;

static const uint8_t NUMBER_OF_BANKS = 100;
static const uint8_t NUMBER_OF_BUTTONS = 6;
static const uint8_t NUMBER_OF_MIDI_MSG = 16;
static const uint8_t BANK_NAME_LENGTH = 9;
static const uint8_t BUTTON_NAME_LENGTH = 10;

#endif