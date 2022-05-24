#include "PushButton.h"

PushButton::PushButton(const uint8_t pin)
{
    this->pin   = pin;
    lastReading = HIGH;
    pinMode(pin, INPUT_PULLUP);
    update();
}

uint8_t PushButton::getState()
{
    update();
    return state;
}

bool PushButton::isPressed()
{
    return (getState() == LOW);
}

void PushButton::update()
{
    uint8_t newReading = digitalRead(pin);

    if (newReading != lastReading)
    {
        lastDebounceTime = millis();
    }
    if (millis() - lastDebounceTime > debounceDelay)
    {
        state = newReading;
    }
    lastReading = newReading;
}
