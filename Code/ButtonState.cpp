#include "ButtonState.h"

ButtonState::ButtonState(const byte firstPinNbr)
{
  state = 0;
  for (byte i = 0; i < NUMBER_OF_PUSHBUTTONS; i++)
  {
    buttons[i] = new PushButton(firstPinNbr + i); // Pins used for buttons must be in consecutive order
  }
}

bool ButtonState::isBankDownState()
{
  bool result = false;
  determineState();
  result = (state == 3 && buttons[0]->isPressed() && buttons[1]->isPressed());
  if (result)
    state = 5;

  return result;
}

bool ButtonState::isBankUpState()
{
  bool result = false;
  determineState();
  result = (state == 3 && buttons[1]->isPressed() && buttons[2]->isPressed());
  if (result)
    state = 5;

  return result;
}

byte ButtonState::getSingleButtonPressed()
{
  determineState();
  if (state == 2)
  {
    state = 5;
    return getButtonPressed();
  }
  else
    return 0;
}

byte ButtonState::getButtonPressed()
{
  for (byte i = 0; i < NUMBER_OF_PUSHBUTTONS; i++)
  {
    if (buttons[i]->isPressed())
      return i + 1;
  }
  return 0;
}

/*
void ButtonState::setButtonsPressedArray()
{
  for (byte i = 0; i < NUMBER_OF_PUSHBUTTONS; i++)
  {
    if (buttons[i]->isPressed())
      btnsPressedArray[i] = true;
    else
      btnsPressedArray[i] = false;
  }
}
*/
void ButtonState::determineState()
{
  /*
   * States:
   * 0: No buttons pressed
   * 1: At least one button pressed
   * 2: Only one button pressed
   * 3: Two buttons pressed
   * 4: More than two buttons pressed. This is an "invalid" state. Now we wait for a return to state 0 (zero)
   * 5: A state has been returned (bankUp/bankDown/btnPressed). Now we wait for a return to state 0 (zero)
   */
  byte cnt = 0;
  switch(state) {
    case 0:
      if (getButtonPressed() > 0) 
      {
        state = 1;
        lastTime = millis();
      }
      break;
    case 1:
      for (byte i = 0; i < NUMBER_OF_PUSHBUTTONS; i++)   
      {
        if (buttons[i]->isPressed())
          cnt++;
      }
      if (millis() - lastTime > stateDelay) 
      {
        if (cnt == 1) 
          state = 2;
        else if (cnt == 2)
          state = 3;

        if (state > 1)
          lastTime = millis();
      }
      break;
    case 2: 
    case 3:
      break;
    case 4:
    case 5:
      for (byte i = 0; i < NUMBER_OF_PUSHBUTTONS; i++)   
      {
        if (buttons[i]->isPressed())
          cnt++;
      }
      if (millis() - lastTime > stateDelay) 
      {
        if (cnt == 0)
          state = 0;
      }
      break;
    //default:
  }
}
