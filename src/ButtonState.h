#ifndef DWARFMASTER_BUTTON_STATE_H
#define DWARFMASTER_BUTTON_STATE_H

#include "PushButton.h"
#include <Arduino.h>

class ButtonState
{
 public:
    static ButtonState* getInstance();

    bool isBankDownState();
    bool isBankUpState();
    bool isButtonJustReleased(uint8_t btnNbr);               // Index starts at 1
    uint8_t getSingleButtonPressed(bool resetState = false); // Index starts at 1

 private:
    ButtonState(); // Singleton
    void determineState();
    uint8_t getButtonPressed();

    static ButtonState* instance;
    static const byte NUMBER_OF_PUSHBUTTONS = 6;
    PushButton* buttons[NUMBER_OF_PUSHBUTTONS];
    uint8_t state;
    uint64_t lastTime               = 0;
    const uint8_t STATE_DELAY       = 50;
    const uint8_t FIRST_PIN_NBR     = 33;
    uint8_t bankChangeButtonPressed = 0;
    uint8_t lastSingleButtonPressed = 0;
};

#endif
