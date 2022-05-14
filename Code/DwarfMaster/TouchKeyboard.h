#ifndef DWARFMASTER_TOUCH_KEYBOARD_H
#define DWARFMASTER_TOUCH_KEYBOARD_H

#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include "Touch.h"
#include "ButtonState.h"


class TouchKeyboard
{
  public: 
    TouchKeyboard(MCUFRIEND_kbv* tft, Touch* touch);
    String getInputFromUser(const String* contextLabel, const String* oldText, const byte maxLength);
    
  private: 
    void drawKeyboard(const String* contextLabel);
    void drawKeyboardLabels(bool letters);
    void drawKeyboardInputText(const String* txt);
    void replaceKeyboardInputText(int pos, const String* txt);
    void drawCursor(byte pos);
    int moveCursor(bool forward, int currentPos, int maxPos);
    String mapPointToKeyboardButton(Point point, bool letters);
    
    const uint16_t DISPLAY_WIDTH         = 320;
    const uint16_t DISPLAY_HEIGHT        = 240;

    const String SPACE_BTN = "[]";
    const String LETTERS_BTN = "ABC";
    const String NUMBERS_BTN = "123";
    const String CURSOR_BACK_BTN = "<-";
    const String CURSOR_FORWARD_BTN = "->";
    const String NOT_A_BUTTON = "not_a_button";
    const String LETTERS[30] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", SPACE_BTN, NUMBERS_BTN, CURSOR_BACK_BTN, CURSOR_FORWARD_BTN};
    const String NUMBERS[30] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "_", "#", "*", "<", ">", "(", ")", "=", "+", "/", "!", "&", "?", "$", "%", SPACE_BTN, LETTERS_BTN, CURSOR_BACK_BTN, CURSOR_FORWARD_BTN};

    const uint16_t KEYBOARD_INPUT_TXT_X = 130;
    const uint16_t KEYBOARD_INPUT_TXT_Y = 10;
    const uint16_t KEYBOARD_INPUT_FONT_WIDTH = 18;

    MCUFRIEND_kbv* tft;
    Touch* touch;
    ButtonState* btnState = btnState->getInstance();
};

#endif
