#ifndef DWARFMASTER_SCREEN_H
#define DWARFMASTER_SCREEN_H

#include <TouchScreen.h>
#include "GlobalConst.h"
#include "Touch.h"
#include "TouchKeyboard.h"
#include <Arduino.h>
#include <MCUFRIEND_kbv.h>

struct EditLabelAttributes
{
    char label1[10];
    char label2[20];
    uint16_t color; 
};

class Screen
{
 public:
    Screen();
    void blankScreen();
    void drawPlayBank(const char* bnkNbr, const char* bnkName, const bool inverted = false);
    void drawPlayButton(const uint8_t btnNbr, const bool patch, const bool selectedPatch, const bool btnStateFirst, const char* btnLabel, bool invertSelected = false);
    bool playBankPressed();

    int getButtonPushed();
    void waitForButtonReleased();
    void drawEdit(const char* label, const char** btnLabels);
    void drawEdit(const char* label, const EditLabelAttributes* btnLabels);
    
    String getKeyboardInputFromUser(const String* contextLabel, const String* oldText, const byte maxLength);
    uint16_t getNumKeyboardInputFromUser(const String* contextLabel, const String* oldVal, const byte maxLength);
    bool getBinaryInputFromUser(const char* label1, const char* label2, const bool firstIsTrue);
    

 private:
    void printButtonLabel(const uint16_t offsetX, const uint16_t offsetY, const char* btnLabel);

    void drawEditGrid();
    void printEditLabel(const char* label);
    void printEditButtonLabels(const char** labels);
    void printEditButtonLabels(const EditLabelAttributes* labels);
    void drawBinaryEdit(const char* label1, const char* label2, bool firstIsTrue);
    uint8_t mapPointToBtn(Point point);

    const uint16_t LINE_WIDTH            = 5;
    const uint16_t PLAY_BTN_LABEL_HEIGHT = 60;
    const uint16_t PLAY_BTN_LABEL_WIDTH  = 104;

    const uint16_t BTN_OFFSET_X_1   = 5;
    const uint16_t BTN_OFFSET_X_2   = BTN_OFFSET_X_1 + PLAY_BTN_LABEL_WIDTH + LINE_WIDTH;
    const uint16_t BTN_OFFSET_X_3   = BTN_OFFSET_X_2 + PLAY_BTN_LABEL_WIDTH + LINE_WIDTH;
    const uint16_t BTN_OFFSET_Y_TOP = 5;
    const uint16_t BTN_OFFSET_Y_BTM = DISPLAY_HEIGHT - PLAY_BTN_LABEL_HEIGHT + BTN_OFFSET_Y_TOP;
    const uint16_t BANK_OFFSET_Y    = 90;

    const uint16_t X_SIZE = 106;
    const uint16_t Y_SIZE = 60;
    
    MCUFRIEND_kbv tft;
    Touch touch;
    TouchKeyboard alphaTouchKeyb = TouchKeyboard(&tft, &touch, true);
    TouchKeyboard numTouchKeyb = TouchKeyboard(&tft, &touch, false);
};

#endif
