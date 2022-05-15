#include "PushButton.h"

PushButton::PushButton(const byte pin) {
  this->pin = pin;
  lastReading = HIGH;
  pinMode(pin, INPUT_PULLUP);
  update();
}

byte PushButton::getState() {
  update();
  return state;
}

bool PushButton::isPressed() {
  return (getState() == LOW);
}

void PushButton::update() {
    // You can handle the debounce of the button directly
    // in the class, so you don't have to think about it
    // elsewhere in your code
    byte newReading = digitalRead(pin);
    
    if (newReading != lastReading) {
      lastDebounceTime = millis();
    }
    if (millis() - lastDebounceTime > debounceDelay) {
      // Update the 'state' attribute only if debounce is checked
      state = newReading;
    }
    lastReading = newReading;
}
