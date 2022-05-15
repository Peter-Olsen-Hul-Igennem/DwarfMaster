#include "Screen.h"


Screen::Screen()
{
  tft.begin(0x9341);
  tft.setRotation(1);            //landscape
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

void Screen::drawPlayButton(const uint8_t btnNbr, const bool patch, const bool selectedPatch, const bool btnStateFirst, const char* btnLabel)
{
  uint16_t bgColor = TFT_LIGHTGREY;
  uint16_t txtColor = TFT_BLACK;
  if (patch)
  {
    if (selectedPatch)
    {
      if (btnStateFirst)
      {
        bgColor = TFT_BLUE;
        txtColor = TFT_WHITE;
      } else {
        bgColor = TFT_RED;
        txtColor = TFT_WHITE;
      }
    }
  } else { // Toggle
    if (btnStateFirst)
    {
      bgColor = TFT_BLACK;
      txtColor = TFT_WHITE;
    } else {
      bgColor = TFT_CYAN;
      txtColor = TFT_BLACK;
    }
  }

  tft.setTextColor(txtColor);
  tft.setTextSize(3);
  //tft.setFont(&FreeSans9pt7b);

  switch (btnNbr) {
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
  size_t labelSize = strlen(btnLabel);
  char txt[maxNbrOfChars + 1];
  if (labelSize < maxNbrOfChars + 1) {
    tft.setCursor(offsetX, offsetY + 14);
    tft.print(String(btnLabel));
  } else {
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
  uint16_t bgColor = TFT_BLACK;
  uint16_t txtColor = TFT_WHITE;
  if (inverted)
  {
    bgColor = TFT_WHITE;
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
  return tKeyb.getInputFromUser(contextLabel, oldText, maxLength);
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
