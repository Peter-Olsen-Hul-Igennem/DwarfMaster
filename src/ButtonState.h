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
    byte getSingleButtonPressed();

 private:
    ButtonState(); // Singleton
    void determineState();
    byte getButtonPressed();

    static ButtonState* instance;
    static const byte NUMBER_OF_PUSHBUTTONS = 6;
    PushButton* buttons[NUMBER_OF_PUSHBUTTONS];
    byte state;
    unsigned long lastTime         = 0;
    const unsigned long stateDelay = 50;
    const byte FIRST_PIN_NBR       = 33;
};

#endif
