#include "ButtonState.h"

ButtonState* ButtonState::instance = nullptr;
;

ButtonState* ButtonState::getInstance()
{
    if (instance == nullptr)
        instance = new ButtonState();

    return instance;
}

ButtonState::ButtonState()
{
    state = 0;
    for (uint8_t i = 0; i < NUMBER_OF_PUSHBUTTONS; i++)
    {
        buttons[i] = new PushButton(FIRST_PIN_NBR + i); // Pins used for buttons must be in consecutive order
    }
}


bool ButtonState::isBankDownState()
{
    determineState();
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
    if (state == 4) 
    {
        state = 6;
        return true;
    }

    return false;
}

uint8_t ButtonState::getSingleButtonPressed(bool resetState)
{
    determineState();
    if (state == 2)
    {
        state = 6;
        lastSingleButtonPressed = getButtonPressed();
        return lastSingleButtonPressed;
    }
    
    if (resetState && state > 2) // Used after bank change to force two pushes of the same button before sending patch midi data
        state = 6;
     
    return 0;
}

bool ButtonState::isButtonJustReleased(uint8_t btnNbr)
{
    if (btnNbr == lastSingleButtonPressed && !buttons[btnNbr - 1]->isPressed())
    {
        lastSingleButtonPressed = 0;
        return true;
    }
    return false;
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
    uint8_t cnt = 0;
    switch (state)
    {
        case 0:
            if (getButtonPressed() > 0)
            {
                state    = 1;
                lastTime = millis();
            }
            break;

        case 1:
            for (uint8_t i = 0; i < NUMBER_OF_PUSHBUTTONS; i++)
            {
                if (buttons[i]->isPressed())
                    cnt++;
            }
            if (millis() - lastTime > STATE_DELAY)
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
            for (uint8_t i = 0; i < NUMBER_OF_PUSHBUTTONS; i++)
            {
                if (buttons[i]->isPressed())
                    cnt++;
            }
            if (millis() - lastTime > STATE_DELAY)
            {
                if (cnt == 0)
                    state = 0;
            }
            break;
    }
}

uint8_t ButtonState::getButtonPressed()
{
    for (uint8_t i = 0; i < NUMBER_OF_PUSHBUTTONS; i++)
    {
        if (buttons[i]->isPressed())
            return i + 1;
    }
    return 0;
}
