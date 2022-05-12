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
    tft.setCursor(offsetX, offsetY+28);
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
/*
void Screen::drawSelectedPatch(const char* patchName)
{
  int y = 140;
  tft.setCursor(5, y);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(4);
  tft.print(String(patchName));
}*/

/*
  void drawPlayScreen()
  {
    int x = PLAY_BTN_LABEL_HEIGHT;
    int y = DISPLAY_HEIGHT - PLAY_BTN_LABEL_HEIGHT - 3;
    int firstVert = PLAY_BTN_LABEL_WIDTH;
    int secondVert = PLAY_BTN_LABEL_WIDTH * 2 + LINE_WIDTH;

    for(int i=0; i<LINE_WIDTH; i++)
    {
        tft.drawLine(0, x + i, DISPLAY_WIDTH, x + i, BLACK); // Top line
        tft.drawLine(0, y + i, DISPLAY_WIDTH, y + i, BLACK); // Btm line
        tft.drawLine(firstVert + i, 0, firstVert + i, PLAY_BTN_LABEL_HEIGHT, BLACK);  // 1. top vertical line
        tft.drawLine(secondVert + i, 0, secondVert + i, PLAY_BTN_LABEL_HEIGHT, BLACK);  // 2. top vertical line
        tft.drawLine(firstVert + i, y, firstVert + i, DISPLAY_HEIGHT, BLACK);  // 1. btm vertical line
        tft.drawLine(secondVert + i, y, secondVert + i, DISPLAY_HEIGHT, BLACK);  // 2. btm vertical line
    }
  }
*/
