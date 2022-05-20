#include "TouchKeyboard.h"
#include "GlobalConst.h"

TouchKeyboard::TouchKeyboard(MCUFRIEND_kbv* tft, Touch* touch, bool fullAlpha)
{
    this->touch     = touch;
    this->tft       = tft;
    this->fullAlpha = fullAlpha;
}

String TouchKeyboard::getInputFromUser(const String* contextLabel, const String* oldText, const byte maxLength)
{
    char tempText[maxLength + 1];
    oldText->toCharArray(tempText, sizeof tempText);

    drawKeyboard(contextLabel);
    drawKeyboardInputText(oldText);

    int pos = 0;
    drawCursor(pos);

    touch->waitForTouchReleased();

    uint16_t releaseCounter = 0;
    String character;
    String prevCharacter = "";
    bool btnReleased     = true;
    Point p;
    bool letters = true;
    bool edit    = true;
    while (edit)
    {
        p = touch->getTouchPoint();
        if (p.pressed)
        {
            character = mapPointToKeyboardButton(p, letters);
            if (btnReleased && character.compareTo(prevCharacter) != 0)
            {
                btnReleased    = false;
                releaseCounter = 0;
                prevCharacter  = character;
                if (character.length() == 1)
                {
                    if (tempText[pos] == '\0')
                        tempText[pos + 1] = '\0';
                    tempText[pos] = character.charAt(0);

                    replaceKeyboardInputText(pos, &character);
                    pos = moveCursor(true, pos, maxLength - 1);
                }
                else if (character.compareTo(SPACE_BTN) == 0)
                {
                    tempText[pos] = ' ';
                    String s      = " ";
                    replaceKeyboardInputText(pos, &s);
                    pos = moveCursor(true, pos, maxLength - 1);
                }
                else if (character.compareTo(CURSOR_BACK_BTN) == 0)
                {
                    pos = moveCursor(false, pos, maxLength - 1);
                }
                else if (character.compareTo(CURSOR_FORWARD_BTN) == 0)
                {
                    pos = moveCursor(true, pos, maxLength - 1);
                }
                else if (character.compareTo(LETTERS_BTN) == 0)
                {
                    letters = true;
                    drawKeyboardLabels(letters);
                }
                else if (character.compareTo(NUMBERS_BTN) == 0)
                {
                    letters = false;
                    drawKeyboardLabels(letters);
                }
            }
            else
            {
                releaseCounter = 0;
            }
        }
        else
        {
            releaseCounter++;
            if (releaseCounter > 500)
            {
                prevCharacter = "";
                btnReleased   = true;
            }
        }
        switch (btnState->getSingleButtonPressed())
        {
            case 1: // CANCEL
                edit = false;
                oldText->toCharArray(tempText, sizeof tempText);
                break;
            case 3: // SAVE
                edit = false;
                break;
        }
    }
    return tempText;
}

void TouchKeyboard::drawKeyboard(const String* contextLabel)
{
    tft->fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, BACKGROUND_COLOR);

    int x;
    int y;
    if (fullAlpha) // Alphanumeric keyboard
    {
        for (int j = 1; j < 6; j++)
        {
            x = X_SIZE_ALPHA * j;
            y = Y_SIZE_ALPHA * j;
            for (int i = 0; i < 3; i++)
            {
                tft->drawLine(0, y + i, DISPLAY_WIDTH, y + i, LINE_COLOR);             // horizontal
                tft->drawLine(x + i, Y_SIZE_ALPHA, x + i, DISPLAY_HEIGHT, LINE_COLOR); // vertical
            }
        }
    }
    else
    { // Numeric keys only
        for (int j = 1; j < 5; j++)
        {
            x = X_SIZE_NUM * j;
            y = Y_SIZE_ALPHA + (Y_SIZE_NUM * (j - 1));
            for (int i = 0; i < 3; i++)
            {
                tft->drawLine(0, y + i, DISPLAY_WIDTH, y + i, LINE_COLOR); // horizontal
                if (i < 2)
                    tft->drawLine(x + i, Y_SIZE_ALPHA, x + i, DISPLAY_HEIGHT, LINE_COLOR); // vertical
            }
        }
    }

    tft->setTextColor(TFT_BLACK);
    tft->setTextSize(2);
    tft->setCursor(10, 15);
    tft->print(*contextLabel + String(":"));
    tft->setTextSize(2);

    drawKeyboardLabels(true);
}

void TouchKeyboard::drawKeyboardLabels(bool letters)
{
    tft->setTextColor(TFT_BLACK);
    tft->setTextSize(2);
    byte cnt = 0;
    int y    = 0;
    int x    = 0;
    if (fullAlpha)
    {
        for (int j = 1; j < 6; j++)
        {
            y = 15 + (Y_SIZE_ALPHA * j);
            for (int i = 0; i < 6; i++)
            {
                x = 10 + (X_SIZE_ALPHA * i);
                tft->fillRect(x, y, Y_SIZE_ALPHA, 20, BACKGROUND_COLOR);

                if (letters)
                {
                    if (LETTERS[cnt].length() == 1)
                        x += 12;
                    else if (LETTERS[cnt].length() == 2)
                        x += 6;

                    tft->setCursor(x, y);
                    tft->print(LETTERS[cnt]);
                }
                else
                {
                    if (NUMBERS_30[cnt].length() == 1)
                        x += 12;
                    else if (NUMBERS_30[cnt].length() == 2)
                        x += 6;

                    tft->setCursor(x, y);
                    tft->print(NUMBERS_30[cnt]);
                }
                cnt++;
            }
        }
    }
    else // Numeric keys only
    {
        for (int j = 1; j < 5; j++)
        {
            y = 10 + (Y_SIZE_NUM * j);
            for (int i = 0; i < 3; i++)
            {
                x = 35 + (X_SIZE_NUM * i);
                tft->fillRect(x, y, Y_SIZE_NUM, 20, BACKGROUND_COLOR);

                if (NUMBERS_12[cnt].length() == 1)
                    x += 12;
                else if (NUMBERS_12[cnt].length() == 2)
                    x += 6;

                tft->setCursor(x, y);
                tft->print(NUMBERS_12[cnt]);

                cnt++;
            }
        }
    }
}

void TouchKeyboard::drawKeyboardInputText(const String* txt)
{
    tft->fillRect(KEYBOARD_INPUT_TXT_X, 0, DISPLAY_WIDTH - KEYBOARD_INPUT_TXT_X, Y_SIZE_ALPHA, BACKGROUND_COLOR);
    tft->setTextColor(TFT_BLACK);
    tft->setTextSize(3);
    tft->setCursor(KEYBOARD_INPUT_TXT_X, KEYBOARD_INPUT_TXT_Y);
    tft->print(*txt);
}

void TouchKeyboard::replaceKeyboardInputText(int pos, const String* txt)
{
    int x = KEYBOARD_INPUT_TXT_X + (KEYBOARD_INPUT_FONT_WIDTH * pos);
    tft->fillRect(x, 0, KEYBOARD_INPUT_FONT_WIDTH, 33, BACKGROUND_COLOR);
    tft->setTextColor(TFT_BLACK);
    tft->setTextSize(3);
    tft->setCursor(x, KEYBOARD_INPUT_TXT_Y);
    tft->print(*txt);
}

int TouchKeyboard::moveCursor(bool forward, int currentPos, int maxPos)
{
    if (forward)
    {
        if (currentPos < maxPos)
        {
            currentPos++;
            drawCursor(currentPos);
        }
    }
    else
    {
        if (currentPos > 0)
        {
            currentPos--;
            drawCursor(currentPos);
        }
    }
    return currentPos;
}

void TouchKeyboard::drawCursor(byte pos)
{
    const int x            = KEYBOARD_INPUT_TXT_X + (KEYBOARD_INPUT_FONT_WIDTH * pos);
    const int y            = 34;
    const int cursorLength = 14;

    tft->fillRect(KEYBOARD_INPUT_TXT_X, y, DISPLAY_WIDTH - KEYBOARD_INPUT_TXT_X, 3, BACKGROUND_COLOR);

    for (int i = 0; i < 3; i++)
    {
        tft->drawLine(x, y + i, x + cursorLength, y + i, TFT_BLACK);
    }
}

String TouchKeyboard::mapPointToKeyboardButton(Point point, bool letters)
{
    if (point.y < Y_SIZE_ALPHA)
        return NOT_A_BUTTON;

    if (fullAlpha)
        return mapPointToAlphaButton(point, letters);
    else
        return mapPointToNumButton(point);
}

String TouchKeyboard::mapPointToAlphaButton(Point point, bool letters)
{
    int x = point.x / X_SIZE_ALPHA + 1;
    int y = (point.y - Y_SIZE_ALPHA) / Y_SIZE_ALPHA;
    int i = x + (y * 6) - 1;
    if (i < 0 || i > 29)
        return NOT_A_BUTTON;

    if (letters)
        return LETTERS[i];

    return NUMBERS_30[i];
}

String TouchKeyboard::mapPointToNumButton(Point point)
{
    int x = point.x / X_SIZE_NUM + 1;
    int y = (point.y - Y_SIZE_NUM) / Y_SIZE_NUM;
    int i = x + (y * 3) - 1;
    if (i < 0 || i > 11)
        return NOT_A_BUTTON;

    return NUMBERS_12[i];
}