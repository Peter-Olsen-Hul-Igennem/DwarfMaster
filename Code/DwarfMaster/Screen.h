#ifndef DWARFMASTER_SCREEN_H
#define DWARFMASTER_SCREEN_H

#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include "Touch.h"
#include "TouchKeyboard.h"


class Screen
{
  public:
    Screen();
    void blankScreen();
    void drawPlayBank(const char* bnkNbr, const char* bnkName, const bool inverted = false);
    //void drawSelectedPatch(const char* patchName);
    void drawPlayButton(const uint8_t btnNbr, const bool patch, const bool selectedPatch, const bool btnStateFirst, const char* btnLabel);

    bool playBankPressed();

    String getKeyboardInputFromUser(const String* contextLabel, const String* oldText, const byte maxLength);

  private:
    void printButtonLabel(const uint16_t offsetX, const uint16_t offsetY, const char* btnLabel);


    const uint16_t LINE_WIDTH            = 5;
    const uint16_t DISPLAY_WIDTH         = 320;
    const uint16_t DISPLAY_HEIGHT        = 240;
    const uint16_t PLAY_BTN_LABEL_HEIGHT = 60;//38;
    const uint16_t PLAY_BTN_LABEL_WIDTH  = 104;

    const uint16_t BTN_OFFSET_X_1 = 5;
    const uint16_t BTN_OFFSET_X_2 = BTN_OFFSET_X_1 + PLAY_BTN_LABEL_WIDTH + LINE_WIDTH;
    const uint16_t BTN_OFFSET_X_3 = BTN_OFFSET_X_2 + PLAY_BTN_LABEL_WIDTH + LINE_WIDTH;
    const uint16_t BTN_OFFSET_Y_TOP = 5;
    const uint16_t BTN_OFFSET_Y_BTM = DISPLAY_HEIGHT - PLAY_BTN_LABEL_HEIGHT + BTN_OFFSET_Y_TOP;

    const uint16_t BANK_OFFSET_Y = 90;

    MCUFRIEND_kbv tft;
    Touch touch;
    TouchKeyboard tKeyb = TouchKeyboard(&tft, &touch);
};

#endif
