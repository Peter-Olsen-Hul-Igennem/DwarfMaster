#ifndef PUSH_BUTTON_H
#define PUSH_BUTTON_H

#include <Arduino.h>

class PushButton {
  public:
    PushButton(const byte pin);
    byte getState();
    bool isPressed();
  private:
    //void init();
    void update();
    
    byte pin;
    byte state;
    byte lastReading;
    unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 20;
};



#endif
