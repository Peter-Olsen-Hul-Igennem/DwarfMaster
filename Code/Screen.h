#ifndef DWARFMASTER_SCREEN_H
#define DWARFMASTER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>



class Screen
{
	public: 
	  Screen();
    void drawBank(const char* bnkNbr, const char* bnkName, const bool inverted = false);
    //void drawSelectedPatch(const char* patchName);
    void drawButtonRect(const uint8_t btnNbr, const bool patch, const bool selectedPatch, const bool btnStateFirst, const char* btnLabel);
    
 	private: 
    const uint16_t LINE_WIDTH            = 5;
    const uint16_t DISPLAY_WIDTH         = 320;
    const uint16_t DISPLAY_HEIGHT        = 240;
    const uint16_t PLAY_BTN_LABEL_HEIGHT = 60;//38;
    const uint16_t PLAY_BTN_LABEL_WIDTH  = 104;

    uint16_t offsetX_1 = 5;
    uint16_t offsetX_2 = offsetX_1 + PLAY_BTN_LABEL_WIDTH + LINE_WIDTH;
    uint16_t offsetX_3 = offsetX_2 + PLAY_BTN_LABEL_WIDTH + LINE_WIDTH;
    uint16_t offsetYTop = 5;
    uint16_t offsetYBtm = DISPLAY_HEIGHT - PLAY_BTN_LABEL_HEIGHT + offsetYTop;

    void printButtonLabel(const uint16_t offsetX, const uint16_t offsetY, const char* btnLabel);
    
    
    MCUFRIEND_kbv tft;


};

#endif
