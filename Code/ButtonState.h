#ifndef BUTTON_STATE_H
#define BUTTON_STATE_H

#include "PushButton.h"
#include <Arduino.h>
          

class ButtonState {
  public:
    ButtonState(const byte firstPinNbr);

    bool isBankDownState();
    bool isBankUpState();
    byte getSingleButtonPressed(); 
    
  private:
    void determineState();
    byte getButtonPressed(); 
    //void setButtonsPressedArray(); 
    
    static const byte NUMBER_OF_PUSHBUTTONS = 6;
    PushButton* buttons[NUMBER_OF_PUSHBUTTONS];
    //bool btnsPressedArray[NUMBER_OF_PUSHBUTTONS];    
    byte state;
    unsigned long lastTime = 0;
    const unsigned long stateDelay = 50;
};



#endif
