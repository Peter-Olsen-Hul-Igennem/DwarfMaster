#ifndef DWARFMASTER_TOUCH_KEYBOARD_H
#define DWARFMASTER_TOUCH_KEYBOARD_H

#include "ButtonState.h"
#include "Touch.h"
#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

class TouchKeyboard
{
 public:
    TouchKeyboard(MCUFRIEND_kbv* tft, Touch* touch, bool fullAlpha);
    String getInputFromUser(const String* contextLabel, const String* oldText, const byte maxLength);

 private:
    void drawKeyboard(const String* contextLabel);
    void drawKeyboardLabels(const bool letters);
    void drawKeyboardInputText(const String* txt);
    void replaceKeyboardInputText(const uint8_t pos, const String* txt);
    void drawCursor(const uint8_t pos);
    int moveCursor(const bool forward, uint8_t currentPos, const uint8_t maxPos);
    String mapPointToKeyboardButton(const Point point, const bool letters);
    String mapPointToAlphaButton(const Point point, const bool letters);
    String mapPointToNumButton(const Point point);

    const String SPACE_BTN          = "[]";
    const String LETTERS_BTN        = "ABC";
    const String NUMBERS_BTN        = "123";
    const String CURSOR_BACK_BTN    = "<-";
    const String CURSOR_FORWARD_BTN = "->";
    const String NOT_A_BUTTON       = "not_a_button";
    const String LETTERS[30]        = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", SPACE_BTN, NUMBERS_BTN, CURSOR_BACK_BTN, CURSOR_FORWARD_BTN};
    const String NUMBERS_30[30]     = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "_", "#", "*", "<", ">", "(", ")", "=", "+", "/", "!", "&", "?", "$", "%", SPACE_BTN, LETTERS_BTN, CURSOR_BACK_BTN, CURSOR_FORWARD_BTN};
    const String NUMBERS_12[12]     = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", CURSOR_BACK_BTN, CURSOR_FORWARD_BTN};

    const uint16_t KEYBOARD_INPUT_TXT_X      = 130;
    const uint16_t KEYBOARD_INPUT_TXT_Y      = 10;
    const uint16_t KEYBOARD_INPUT_FONT_WIDTH = 18;
    const uint16_t X_SIZE_ALPHA              = 53;
    const uint16_t Y_SIZE_ALPHA              = 40;
    const uint16_t X_SIZE_NUM                = 106;
    const uint16_t Y_SIZE_NUM                = 50;

    MCUFRIEND_kbv* tft;
    Touch* touch;
    ButtonState* btnState = btnState->getInstance();
    bool fullAlpha;
};

#endif
