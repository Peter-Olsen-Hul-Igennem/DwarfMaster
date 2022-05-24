#ifndef DWARFMASTER_PUSH_BUTTON_H
#define DWARFMASTER_PUSH_BUTTON_H

#include <Arduino.h>

class PushButton
{
 public:
    PushButton(const uint8_t pin);
    uint8_t getState();
    bool isPressed();

 private:
    void update();

    uint8_t pin;
    uint8_t state;
    uint8_t lastReading;
    uint64_t lastDebounceTime = 0;
    uint8_t debounceDelay     = 20;
};

#endif
