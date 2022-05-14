#include "Screen.h"


Screen::Screen()
{
  tft.begin(0x9341);
  tft.setRotation(1);            //landscape
  tft.fillScreen(TFT_BLACK);
}

void Screen::drawButtonRect(const uint8_t btnNbr, const bool patch, const bool selectedPatch, const bool btnStateFirst, const char* btnLabel)
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
      tft.fillRect(0, 0, PLAY_BTN_LABEL_WIDTH, PLAY_BTN_LABEL_HEIGHT, bgColor);
      printButtonLabel(offsetX_1, offsetYTop, btnLabel);
      break;
    case 2:
      tft.fillRect(PLAY_BTN_LABEL_WIDTH + LINE_WIDTH, 0, PLAY_BTN_LABEL_WIDTH, PLAY_BTN_LABEL_HEIGHT, bgColor);
      printButtonLabel(offsetX_2, offsetYTop, btnLabel);
      break;
    case 3:
      tft.fillRect(PLAY_BTN_LABEL_WIDTH + LINE_WIDTH + PLAY_BTN_LABEL_WIDTH + LINE_WIDTH, 0, PLAY_BTN_LABEL_WIDTH, PLAY_BTN_LABEL_HEIGHT, bgColor);
      printButtonLabel(offsetX_3, offsetYTop, btnLabel);
      break;
    case 4:
      tft.fillRect(0, DISPLAY_HEIGHT - PLAY_BTN_LABEL_HEIGHT, PLAY_BTN_LABEL_WIDTH, PLAY_BTN_LABEL_HEIGHT, bgColor);
      printButtonLabel(offsetX_1, offsetYBtm, btnLabel);
      break;
    case 5:
      tft.fillRect(PLAY_BTN_LABEL_WIDTH + LINE_WIDTH, DISPLAY_HEIGHT - PLAY_BTN_LABEL_HEIGHT, PLAY_BTN_LABEL_WIDTH, PLAY_BTN_LABEL_HEIGHT, bgColor);
      printButtonLabel(offsetX_2, offsetYBtm, btnLabel);
      break;
    case 6:
      tft.fillRect(PLAY_BTN_LABEL_WIDTH + LINE_WIDTH + PLAY_BTN_LABEL_WIDTH + LINE_WIDTH, DISPLAY_HEIGHT - PLAY_BTN_LABEL_HEIGHT, PLAY_BTN_LABEL_WIDTH, PLAY_BTN_LABEL_HEIGHT, bgColor);
      printButtonLabel(offsetX_3, offsetYBtm, btnLabel);
    break; default:
      break;
  }

}

void Screen::printButtonLabel(const uint16_t offsetX, const uint16_t offsetY, const char* btnLabel)
{
  size_t maxNbrOfChars = 5;
  size_t labelSize = strlen(btnLabel);
  Serial.println(labelSize);
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


void Screen::drawBank(const char* bnkNbr, const char* bnkName, const bool inverted)
{
  uint16_t bgColor = TFT_BLACK;
  uint16_t txtColor = TFT_WHITE;
  if (inverted)
  {
    bgColor = TFT_WHITE;
    txtColor = TFT_BLACK;
  }
  tft.fillRect(0, 82, DISPLAY_WIDTH, DISPLAY_HEIGHT - 170, bgColor);

  int y = 90;
  tft.setCursor(5, y);
  tft.setTextColor(txtColor);
  tft.setTextSize(8);
  tft.print(String(bnkNbr));

  tft.setCursor(PLAY_BTN_LABEL_WIDTH, y + 28);
  tft.setTextSize(4);
  tft.print(String(bnkName));
}

String Screen::getKeyboardInputFromUser(const String* contextLabel, const String* oldText, const byte maxLength)
{
  return tKeyb.getInputFromUser(contextLabel, oldText, maxLength);
}
