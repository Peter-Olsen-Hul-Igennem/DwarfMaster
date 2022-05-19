#include "Screen.h"
#include "ButtonState.h"


Screen::Screen()
{
    tft.begin(0x9341);
    tft.setRotation(1); // landscape
    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_DARKGREY);
    tft.setTextSize(8);
    tft.setCursor(45, 50);
    tft.print("DWARF");
    tft.setCursor(20, 130);
    tft.print("MASTER");
}

void Screen::blankScreen()
{
    tft.fillScreen(TFT_BLACK);
}

void Screen::showMessage(const char* msg)
{
    tft.fillScreen(BACKGROUND_COLOR);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(3);
    tft.setCursor(10, 45);
    tft.print(String(msg));

    Point p;
    ButtonState* btnState = btnState->getInstance();
    while (true)
    {
        p = touch.getTouchPoint();
        if (p.pressed)
        {
            return;
        }
        if (btnState->getSingleButtonPressed() == 1) // Footswitch 1: cancel
        {
            return;
        }
    }
}

void Screen::drawPlayButton(const uint8_t btnNbr, const bool patch, const bool selectedPatch, const bool btnStateFirst, const char* btnLabel, bool invertSelected)
{
    uint16_t bgColor  = TFT_LIGHTGREY;
    uint16_t txtColor = TFT_BLACK;
    if (patch)
    {
        if (selectedPatch && !invertSelected) 
        {
            if (btnStateFirst)
            {
                bgColor  = TFT_BLUE;
                txtColor = TFT_WHITE;
            }
            else
            {
                bgColor  = TFT_RED;
                txtColor = TFT_WHITE;
            }
        }
    }
    else
    { // Toggle
        if (btnStateFirst)
        {
            bgColor  = TFT_BLACK;
            txtColor = TFT_WHITE;
        }
        else
        {
            bgColor  = TFT_CYAN;
            txtColor = TFT_BLACK;
        }
    }

    tft.setTextColor(txtColor);
    tft.setTextSize(3);

    switch (btnNbr)
    {
        case 1:
            tft.fillRect(0, DISPLAY_HEIGHT - PLAY_BTN_LABEL_HEIGHT, PLAY_BTN_LABEL_WIDTH, PLAY_BTN_LABEL_HEIGHT, bgColor);
            printButtonLabel(BTN_OFFSET_X_1, BTN_OFFSET_Y_BTM, btnLabel);
            break;
        case 2:
            tft.fillRect(PLAY_BTN_LABEL_WIDTH + LINE_WIDTH, DISPLAY_HEIGHT - PLAY_BTN_LABEL_HEIGHT, PLAY_BTN_LABEL_WIDTH, PLAY_BTN_LABEL_HEIGHT, bgColor);
            printButtonLabel(BTN_OFFSET_X_2, BTN_OFFSET_Y_BTM, btnLabel);
            break;
        case 3:
            tft.fillRect(PLAY_BTN_LABEL_WIDTH + LINE_WIDTH + PLAY_BTN_LABEL_WIDTH + LINE_WIDTH, DISPLAY_HEIGHT - PLAY_BTN_LABEL_HEIGHT, PLAY_BTN_LABEL_WIDTH, PLAY_BTN_LABEL_HEIGHT, bgColor);
            printButtonLabel(BTN_OFFSET_X_3, BTN_OFFSET_Y_BTM, btnLabel);
            break;
        case 4:
            tft.fillRect(0, 0, PLAY_BTN_LABEL_WIDTH, PLAY_BTN_LABEL_HEIGHT, bgColor);
            printButtonLabel(BTN_OFFSET_X_1, BTN_OFFSET_Y_TOP, btnLabel);
            break;
        case 5:
            tft.fillRect(PLAY_BTN_LABEL_WIDTH + LINE_WIDTH, 0, PLAY_BTN_LABEL_WIDTH, PLAY_BTN_LABEL_HEIGHT, bgColor);
            printButtonLabel(BTN_OFFSET_X_2, BTN_OFFSET_Y_TOP, btnLabel);
            break;
        case 6:
            tft.fillRect(PLAY_BTN_LABEL_WIDTH + LINE_WIDTH + PLAY_BTN_LABEL_WIDTH + LINE_WIDTH, 0, PLAY_BTN_LABEL_WIDTH, PLAY_BTN_LABEL_HEIGHT, bgColor);
            printButtonLabel(BTN_OFFSET_X_3, BTN_OFFSET_Y_TOP, btnLabel);
            break;

        default:
            break;
    }
}

void Screen::printButtonLabel(const uint16_t offsetX, const uint16_t offsetY, const char* btnLabel)
{
    size_t maxNbrOfChars = 5;
    size_t labelSize     = strlen(btnLabel);
    char txt[maxNbrOfChars + 1];
    if (labelSize < maxNbrOfChars + 1)
    {
        tft.setCursor(offsetX, offsetY + 14);
        tft.print(String(btnLabel));
    }
    else
    {
        tft.setCursor(offsetX, offsetY);
        strncpy(txt, &btnLabel[0], maxNbrOfChars);
        txt[maxNbrOfChars] = '\0';
        tft.print(String(txt));
        tft.setCursor(offsetX, offsetY + 28);
        strncpy(txt, &btnLabel[maxNbrOfChars], maxNbrOfChars);
        txt[maxNbrOfChars] = '\0';
        tft.print(String(txt));
    }
}

void Screen::drawPlayBank(const char* bnkNbr, const char* bnkName, const bool inverted)
{
    uint16_t bgColor  = TFT_BLACK;
    uint16_t txtColor = TFT_WHITE;
    if (inverted)
    {
        bgColor  = TFT_WHITE;
        txtColor = TFT_BLACK;
    }
    tft.fillRect(0, BANK_OFFSET_Y - 10, DISPLAY_WIDTH, DISPLAY_HEIGHT - 160, bgColor);

    tft.setCursor(5, BANK_OFFSET_Y);
    tft.setTextColor(txtColor);
    tft.setTextSize(8);
    tft.print(String(bnkNbr));

    tft.setCursor(PLAY_BTN_LABEL_WIDTH, BANK_OFFSET_Y + 28);
    tft.setTextSize(4);
    tft.print(String(bnkName));
}

String Screen::getKeyboardInputFromUser(const String* contextLabel, const String* oldText, const byte maxLength)
{
    return alphaTouchKeyb.getInputFromUser(contextLabel, oldText, maxLength);
}

uint16_t Screen::getNumKeyboardInputFromUser(const String* contextLabel, const String* oldVal, const byte maxLength)
{
    uint16_t result = 0;
    String newVal = numTouchKeyb.getInputFromUser(contextLabel, oldVal, maxLength);
    result = newVal.toInt();
    return result;
}

bool Screen::playBankPressed()
{
    Point p = touch.getTouchPoint();
    if (p.pressed)
    {
        if (p.y > BANK_OFFSET_Y && p.y < DISPLAY_HEIGHT - BANK_OFFSET_Y)
            return true;
    }
    return false;
}

void Screen::drawEdit(const char* label, const EditLabelAttributes* btnLabels)
{
    drawEditGrid();
    printEditLabel(label);
    printEditButtonLabels(btnLabels);
}

int Screen::getButtonPushed() {
    uint16_t cnt = 0;
    int prevButton = -2;
    int button = -1;

    ButtonState* btnState = btnState->getInstance();
    
    Point p;
    while (true)
    {
        p = touch.getTouchPoint();
        if (p.pressed)
        {
            button = mapPointToBtn(p);
            if (button != prevButton)
            {
                cnt = 0;
                prevButton = button;
            }
            else
            {
                cnt++;
                if (cnt > 100 && button != -1)
                    break;
            }
        }
        if (btnState->getSingleButtonPressed() == 1) // Footswitch 1: cancel
        {
            return 0;
        }
    }
    return button;
}

void Screen::waitForButtonReleased() 
{
    touch.waitForTouchReleased();
    return;
}

void Screen::drawEditGrid()
{
    tft.fillScreen(BACKGROUND_COLOR);
    
    int x;
    int y;
    for (int j = 1; j < 4; j++)
    {
        y = Y_SIZE * j;
        for (int i = 0; i < 3; i++)
        {
            tft.drawLine(0, y + i, DISPLAY_WIDTH, y + i, LINE_COLOR);

        }
        x = X_SIZE * j;
        if (j < 3)
        {
            for (int i = 0; i < 3; i++)
            {
                tft.drawLine(x + i, 60, x + i, DISPLAY_HEIGHT, LINE_COLOR);
            }
        }
    }
}

void Screen::printEditLabel(const char* label)
{
    tft.setCursor(10, 17);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(4);
    tft.print(String(label));
}

void Screen::printEditButtonLabels(const EditLabelAttributes* labels)
{
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    byte cnt = 0;
    int y    = 0;
    int x    = 0;
    for (int j = 1; j < 4; j++)
    {
        y = 15 + (Y_SIZE * j);
        for (int i = 0; i < 3; i++)
        {
            x = 7 + (X_SIZE * i);
            tft.fillRect(x, y, 95, 40, BACKGROUND_COLOR);
            tft.setTextSize(2);
            tft.setCursor(x, y);
            tft.print(labels[cnt].label1);
            tft.setTextSize(1);
            tft.setCursor(x+5, y+25);
            tft.print(labels[cnt].label2);
            if (labels[cnt].color > 0)
                tft.fillRect(x, y+25, 95, 5, labels[cnt].color);
            cnt++;
        }
    }
}

bool Screen::getBinaryInputFromUser(const char* label1, const char* label2, const bool firstIsTrue)
{
    bool prevChoice = firstIsTrue;
    bool newChoice = firstIsTrue;
    
    drawBinaryEdit(label1, label2, firstIsTrue);
    
    waitForButtonReleased();

    ButtonState* btnState = btnState->getInstance();
    Point p;
    while (true)
    {
        p = touch.getTouchPoint();
        if (p.pressed)
        {
            if (p.y < DISPLAY_HEIGHT / 2)
                newChoice = true;
            else
                newChoice = false;

            if (prevChoice != newChoice)
            {
                drawBinaryEdit(label1, label2, newChoice);
                waitForButtonReleased();
                break;
            }
            else
            {
                waitForButtonReleased();
                break;
            }
        }
        if (btnState->getSingleButtonPressed() == 1) // Footswitch 1: cancel
        {
            return firstIsTrue;
        }
    }
    return newChoice;
}
    
void Screen::drawBinaryEdit(const char* label1, const char* label2, bool firstIsTrue)
{
    tft.fillScreen(BACKGROUND_COLOR);
    if (firstIsTrue)
        tft.fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT / 2, TFT_GREEN);
    else
        tft.fillRect(0, DISPLAY_HEIGHT / 2, DISPLAY_WIDTH, DISPLAY_HEIGHT, TFT_RED);

    for (size_t i = 0; i < 5; i++)
    {
        tft.drawLine(0, DISPLAY_HEIGHT / 2 - 2 + i, DISPLAY_WIDTH, DISPLAY_HEIGHT / 2 - 2 + i, TFT_BLACK);
    }
    
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(4);
    tft.setCursor(10, 45);
    tft.print(String(label1));
    tft.setCursor(10, 165);
    tft.print(String(label2));
}

uint8_t Screen::mapPointToBtn(Point point)
{
    if (point.y < Y_SIZE)
        return -1;

    uint8_t x = point.x / X_SIZE + 1;
    uint8_t y = point.y / Y_SIZE - 1;
/*Serial.print(point.x);
    Serial.print(" - ");
    Serial.print(x);
    Serial.print(" | ");
    Serial.print(point.y);
    Serial.print(" - ");
    Serial.println(y);
    
    Serial.println(x + (3 * y));*/
    return x + (3 * y);
}
