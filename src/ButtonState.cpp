#include "ButtonState.h"


ButtonState* ButtonState::instance = nullptr;;

ButtonState* ButtonState::getInstance()
{
  if (instance == nullptr)
    instance = new ButtonState();
      
  return instance;
}

ButtonState::ButtonState()
{
  state = 0;
  for (byte i = 0; i < NUMBER_OF_PUSHBUTTONS; i++)
  {
    buttons[i] = new PushButton(FIRST_PIN_NBR + i); // Pins used for buttons must be in consecutive order
  }
}

void ButtonState::determineState()
{
  /*
   * States:
   * 0: No buttons pressed
   * 1: At least one button pressed
   * 2: Only one button pressed
   * 3: Two buttons pressed: BankUP
   * 4: Two buttons pressed: BankDOWN
   * 5: An "invalid" combiantion of buttons pressed.
   * 6: A state has been returned (bankUp/bankDown/btnPressed). Now we wait for a return to state 0 (zero)
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
        { 
          if (buttons[0]->isPressed() && buttons[1]->isPressed())
            state = 3;
          else if (buttons[1]->isPressed() && buttons[2]->isPressed())
            state = 4;
          else
            state = 5;
        }
        else
          state = 5;
          

        if (state > 1)
          lastTime = millis();
      }
      break;
    
    case 2: 
    case 3: 
    case 4:
      break;
    
    case 5:
    case 6:
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

bool ButtonState::isBankDownState()
{
  determineState();
  //bool result = state == 3; //result = (state == 3 && buttons[0]->isPressed() && buttons[1]->isPressed());
  if (state == 3)
  {
    state = 6;
    return true;
  }
  return false;
}

bool ButtonState::isBankUpState()
{
  determineState();
  if (state == 4)// (state == 3 && buttons[1]->isPressed() && buttons[2]->isPressed());
  {
    state = 6;
    return true;
  }

  return false;
}

byte ButtonState::getSingleButtonPressed()
{
  determineState();
  if (state == 2)
  {
    state = 6;
    return getButtonPressed();
  }

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
