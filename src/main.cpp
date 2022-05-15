#include "Arduino.h"
#include "ButtonState.h"
#include "MidiCtrlData.h"
#include "Screen.h"
#include "Touch.h"
#include <EEPROM.h>

void setup();
void loop();
void playMode();
void editMode();
void bankChange(bool up);
void drawPlayScreen();
void drawPlayBtns(const Bank* bank, const bool overrideSelected);
void drawPlayBank(const int bnkNbr, const Bank* bank, const bool inverted);
void sendBankMidi(Bank* bank);
void sendButtonMidi(Button button, bool stateFirst);
void loadDataStateInfo();
void saveDataStateInfo();

const char* DATA_VERSION        = "0.3";
const int DATA_STATE_EEPROM_ADR = 0;

Screen screen;
MidiCtrlData* mcd;
ButtonState* btnState = btnState->getInstance();

struct DataState
{
    uint8_t bnk;
    uint8_t btn;
    uint8_t btnStateFirst;
};
DataState dataState;

enum CtrlState { PLAY,
                 PLAY_BANK_UP,
                 PLAY_BANK_DOWN,
                 EDIT };
CtrlState ctrlState = PLAY;
// CtrlState prevCtrlState = EDIT;

void setup()
{
    Serial.begin(9600);
    delay(1000);

    loadDataStateInfo();
    mcd = new MidiCtrlData(DATA_VERSION);
    drawPlayScreen();
}

void loop()
{

    // switch that sets up state
    switch (ctrlState)
    {
        case PLAY:
            if (screen.playBankPressed())
            {
                //  prevCtrlState = ctrlState;
                ctrlState = EDIT;
            }
            else if (btnState->isBankUpState())
            {
                //  prevCtrlState = ctrlState;
                ctrlState = PLAY_BANK_UP;
            }
            else if (btnState->isBankDownState())
            {
                //  prevCtrlState = ctrlState;
                ctrlState = PLAY_BANK_DOWN;
            }
            break;

        case PLAY_BANK_UP:
        case PLAY_BANK_DOWN:
            //  prevCtrlState = ctrlState;
            ctrlState = PLAY;
            break;

        case EDIT:
            break;
    }

    /*if (ctrlState != PLAY && prevCtrlState == ctrlState)
    {
      return;
    }*/

    // switch that reacts to a given state
    switch (ctrlState)
    {
        case PLAY:
            playMode();
            break;
        case PLAY_BANK_UP:
            bankChange(true);
            break;
        case PLAY_BANK_DOWN:
            bankChange(false);
            break;
        case EDIT:
            editMode();
            break;
    }

    // prevCtrlState = ctrlState;

    while (usbMIDI.read())
    {
        // ignore incoming messages
    }
}

void playMode()
{
    byte btnNbr = btnState->getSingleButtonPressed();
    if (btnNbr > 0)
    {
        if (btnNbr == dataState.btn && dataState.btnStateFirst == true)
        {
            dataState.btnStateFirst = false;
        }
        else
        {
            dataState.btnStateFirst = true;
        }
        Bank* bank = mcd->getBank(dataState.bnk);
        sendButtonMidi(bank->buttons[btnNbr - 1], dataState.btnStateFirst);
        dataState.btn = btnNbr;
        saveDataStateInfo();
        drawPlayBtns(bank, false);
    }
}

void editMode()
{
    Serial.println("EDIT");
}

void bankChange(bool up)
{
    unsigned long prevMillis = millis();
    unsigned long curMillis;
    const unsigned long period = 500;
    bool bBlink                = false;
    bool first                 = true;
    byte tmpBankNbr            = dataState.bnk;
    Bank* bank                 = mcd->getBank(dataState.bnk); // failsafe initialization

    while (true)
    {
        if ((first && up) || btnState->isBankUpState())
        {
            if (tmpBankNbr == NUMBER_OF_BANKS - 1)
                tmpBankNbr = 0;
            else
                tmpBankNbr++;
            bank = mcd->getBank(tmpBankNbr);
        }
        else if ((first && !up) || btnState->isBankDownState())
        {
            if (tmpBankNbr == 0)
                tmpBankNbr = NUMBER_OF_BANKS - 1;
            else
                tmpBankNbr--;
            bank = mcd->getBank(tmpBankNbr);
        }
        first = false;

        curMillis = millis();
        if (curMillis - prevMillis >= period)
        {
            bBlink = !bBlink;
            drawPlayBank(tmpBankNbr, bank, bBlink);
            drawPlayBtns(bank, true);
            prevMillis = curMillis;
        }
        int btnNbr = btnState->getSingleButtonPressed();
        if (btnNbr > 0 && bank->buttons[btnNbr - 1].isPatch)
        {
            sendBankMidi(bank);
            sendButtonMidi(bank->buttons[btnNbr - 1], dataState.btnStateFirst);
            dataState.bnk           = tmpBankNbr;
            dataState.btn           = btnNbr;
            dataState.btnStateFirst = true;
            saveDataStateInfo();
            drawPlayScreen();

            break;
        }
    }
}

void drawPlayScreen()
{
    screen.blankScreen();
    Bank* bank = mcd->getBank(dataState.bnk);
    drawPlayBtns(bank, false);
    drawPlayBank(dataState.bnk, bank, false);
}

void drawPlayBtns(const Bank* bank, const bool overrideSelected)
{
    byte curPatch    = dataState.btn;
    bool selected    = false;
    bool primaryFunc = true;

    for (byte i = 0; i < NUMBER_OF_BUTTONS; i++)
    {
        if (!overrideSelected)
            selected = curPatch == i + 1;
        if (selected && !dataState.btnStateFirst)
            primaryFunc = false;
        screen.drawPlayButton(i + 1, bank->buttons[i].isPatch, selected, primaryFunc, bank->buttons[i].name);
    }
}

void drawPlayBank(const int bnkNbr, const Bank* bank, const bool inverted)
{
    char bankNbr[3];
    itoa(bnkNbr, bankNbr, 10);
    if (bnkNbr < 10)
    {
        bankNbr[1] = bankNbr[0];
        bankNbr[0] = '0';
    }
    bankNbr[2] = '\0';

    screen.drawPlayBank(bankNbr, bank->name, inverted);
}

void sendBankMidi(Bank* bank)
{
    if (bank->pcMessage.channel > 0)
    {
        usbMIDI.sendProgramChange(bank->pcMessage.valueOn, bank->pcMessage.channel);
    }
}

void sendButtonMidi(Button button, bool stateFirst)
{
    uint8_t val;
    for (uint8_t i = 0; i < NUMBER_OF_MIDI_MSG; i++)
    {
        if (button.pcMessages[i].channel > 0)
        {
            if (stateFirst)
                val = button.pcMessages[i].valueOn;
            else
                val = button.pcMessages[i].valueOff;
        
            usbMIDI.sendProgramChange(val, button.pcMessages[i].channel);
        }

        if (button.ccMessages[i].channel > 0)
        {
            if (stateFirst)
                val = button.ccMessages[i].valueOn;
            else
                val = button.ccMessages[i].valueOff;
        
            usbMIDI.sendControlChange(button.ccMessages[i].ccNumber, val, button.ccMessages[i].channel);
        }
    }
}

void loadDataStateInfo()
{
    EEPROM.get(DATA_STATE_EEPROM_ADR, dataState);
    if (dataState.bnk < 0 || dataState.bnk > NUMBER_OF_BANKS - 1)
        dataState.bnk = 0;
    if (dataState.btn < 1 || dataState.btn > NUMBER_OF_BUTTONS)
    {
        dataState.btn           = 1;
        dataState.btnStateFirst = true;
    }
}

void saveDataStateInfo()
{
    EEPROM.put(DATA_STATE_EEPROM_ADR, dataState);
}
