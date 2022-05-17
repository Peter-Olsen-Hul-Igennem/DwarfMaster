#include "ButtonState.h"
#include "Edit.h"
#include "GlobalConst.h"
#include "MidiCtrlData.h"
#include "Screen.h"
#include "Touch.h"
#include <Arduino.h>
#include <EEPROM.h>

const char* DATA_VERSION        = "0.4";
const int DATA_STATE_EEPROM_ADR = 0;

enum PcCcEnum { PC_1_8,
                PC_9_16,
                CC_1_8,
                CC_9_16 };

void setup();
void loop();

void playMode();
void bankChange(bool up);
void drawPlayScreen();
void drawPlayBtns(const Bank* bank, const bool overrideSelected);
void drawPlayBank(const int bnkNbr, const Bank* bank, const bool inverted);
void sendBankMidi(Bank* bank);
void sendButtonMidi(Button button, bool stateFirst);

void editMode();
bool doEditBank(Bank* bank);
bool doEditBtn(Button* button, uint8_t btnNbr);
bool doPcCcEdit(Button* button, uint8_t btnNbr, PcCcEnum pcCc);
bool doPcCcDetailsEdit(Button* button, uint8_t btnNbr, uint8_t pcCcNbr, PcCcEnum pcCc);

void loadDataStateInfo();
void saveDataStateInfo();
void bankNumToChar(uint8_t num, char* buf);

const char* MAIN_EDIT_LABELS[9]{"  EXIT", "  BANK", "UTILITY", " BTN #4", " BTN #5", " BTN #6", " BTN #1", " BTN #2", " BTN #3"};
const char* BANK_EDIT_LABELS[9]{"  EXIT", "", "", "  NAME", " PC CH", " PC NBR", "", "", ""};
const char* BUTTON_EDIT_LABELS[9]{"  EXIT", "  NAME", " PATCH", " PC 1-8", " PC 9-16", " 2. PUSH", " CC 1-8", " CC 9-16", " TOGL ON"};
const char* PC_1_EDIT_LABELS[9]{"  EXIT", "  PC #1", "  PC #2", "  PC #3", "  PC #4", "  PC #5", "  PC #6", "  PC #7", "  PC #8"};
const char* PC_2_EDIT_LABELS[9]{"  EXIT", "  PC #9", " PC #10", " PC #11", " PC #12", " PC #13", " PC #14", " PC #15", " PC #16"};
const char* CC_1_EDIT_LABELS[9]{"  EXIT", "  CC #1", "  CC #2", "  CC #3", "  CC #4", "  CC #5", "  CC #6", "  CC #7", "  CC #8"};
const char* CC_2_EDIT_LABELS[9]{"  EXIT", "  CC #9", " CC #10", " CC #11", " CC #12", " CC #13", " CC #14", " CC #15", " CC #16"};
const char* PC_DETAIL_EDIT_LABELS[9]{"  EXIT", " PC CH", " PC NBR", "", "", "", "", "", ""};
const char* CC_DETAIL_EDIT_LABELS[9]{"  EXIT", " CC CH", " CC NBR", "", "MIN VAL", "MAX VAL", "", "", ""};

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

    /* bool a = true;
     bool b = false;
     bool c = a || b;
     Serial.println(c);*/
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
            ctrlState = PLAY;
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
    char bankNumber[3];
    bankNumToChar(bnkNbr, bankNumber);
    screen.drawPlayBank(bankNumber, bank->name, inverted);
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
                val = button.ccMessages[i].minValue;
            else
                val = button.ccMessages[i].maxValue;

            usbMIDI.sendControlChange(button.ccMessages[i].ccNumber, val, button.ccMessages[i].channel);
        }
    }
}

/*
 * EDIT functions starts here
 */
void editMode()
{
    Serial.println("EDIT");

    bool dirty = false;
    char label[13];
    bankNumToChar(dataState.bnk, label);
    Bank* bank      = mcd->getBank(dataState.bnk);
    Button* pButton = nullptr;
    label[2]        = ' ';

    int btn;
    bool loop = true;
    do
    {
        for (size_t i = 0; i < BANK_NAME_LENGTH + 1; i++)
        {
            label[3 + i] = bank->name[i];
        }
        screen.drawEdit(label, MAIN_EDIT_LABELS);
        screen.waitForButtonReleased();

        btn = screen.getButtonPushed();
        switch (btn)
        {
            case 1: // EXIT
                loop = false;
                break;
            case 2: // BANK
                dirty = doEditBank(bank) || dirty;
                break;
            case 3: // UTILITY
                break;
            case 4: // BTN #4
            case 5: // BTN #5
            case 6: // BTN #6
            {
                pButton = &bank->buttons[btn - 1];
                dirty   = doEditBtn(pButton, btn) || dirty;
                break;
            }
            case 7: // BTN #1
            case 8: // BTN #2
            case 9: // BTN #3
            {
                pButton = &bank->buttons[btn - 7];
                dirty   = doEditBtn(pButton, btn - 6) || dirty;
                break;
            }
        }
    } while (loop);

    if (dirty)
        mcd->saveBank(dataState.bnk);

    drawPlayScreen();
    screen.waitForButtonReleased();
}

bool doEditBank(Bank* bank)
{
    int btn;
    bool loop  = true;
    bool dirty = false;

    String label;
    String sVal;
    uint8_t nVal;
    uint8_t newVal;
    char bankName[13];
    bankNumToChar(dataState.bnk, bankName);
    bankName[2] = ' ';

    while (loop)
    {
        for (size_t i = 0; i < BANK_NAME_LENGTH + 1; i++)
        {
            bankName[3 + i] = bank->name[i];
        }
        screen.drawEdit(bankName, BANK_EDIT_LABELS);
        screen.waitForButtonReleased();

        btn = screen.getButtonPushed();
        switch (btn)
        {
            case 1: // EXIT
                loop = false;
                break;
            case 2:
            case 3:
                break;
            case 4: // NAME
            {
                label          = String("NAME");
                String name    = String(bank->name);
                String newName = screen.getKeyboardInputFromUser(&label, &name, BANK_NAME_LENGTH);
                if (!name.equals(newName))
                {
                    strcpy(bank->name, newName.c_str());
                    dirty = true;
                }
                break;
            }
            case 5: // PC CH
            {
                label  = String("PC CH");
                nVal   = bank->pcMessage.channel;
                sVal   = String(nVal);
                newVal = screen.getNumKeyboardInputFromUser(&label, &sVal, 2);
                if (newVal > 16)
                    newVal = 16;
                if (nVal != newVal)
                {
                    bank->pcMessage.channel = newVal;
                    dirty                   = true;
                }
                break;
            }
            case 6: // PC NBR
                label  = String("PC NBR");
                nVal   = bank->pcMessage.valueOn;
                sVal   = String(nVal);
                newVal = screen.getNumKeyboardInputFromUser(&label, &sVal, 3);
                if (newVal > 127)
                    newVal = 127;
                if (nVal != newVal)
                {
                    bank->pcMessage.valueOn = newVal;
                    dirty                   = true;
                }

                break;
            case 7:
            case 8:
            case 9:
                break;
        }
    }
    return dirty;
}

bool doEditBtn(Button* button, const uint8_t btnNbr)
{
    int btn;
    bool loop  = true;
    bool dirty = false;

    String label;
    String sVal;
    char label1[16];
    char label2[16];
    char btnName[13];
    itoa(btnNbr, btnName, 10);
    btnName[1] = ' ';

    while (loop)
    {
        for (size_t i = 0; i < BUTTON_NAME_LENGTH + 1; i++)
        {
            btnName[2 + i] = button->name[i];
        }
        screen.drawEdit(btnName, BUTTON_EDIT_LABELS);
        screen.waitForButtonReleased();

        btn = screen.getButtonPushed();
        switch (btn)
        {
            case 1: // EXIT
                loop = false;
                break;
            case 2: // NAME
            {
                label          = String("NAME");
                String name    = String(button->name);
                String newName = screen.getKeyboardInputFromUser(&label, &name, BUTTON_NAME_LENGTH);
                if (!name.equals(newName))
                {
                    strcpy(button->name, newName.c_str());
                    dirty = true;
                }
                break;
            }
            case 3: // PATCH
            {
                strcpy(label1, "   PATCH");
                strcpy(label2, "   TOGGLE");
                bool result = screen.getBinaryInputFromUser(label1, label2, button->isPatch);
                if (result != button->isPatch)
                {
                    button->isPatch = result;
                    dirty           = true;
                }
                break;
            }
            case 4: // PC 1-8
                doPcCcEdit(button, btnNbr, PC_1_8);
                break;
            case 5: // PC 9-16
                doPcCcEdit(button, btnNbr, PC_9_16);
                break;
            case 6: // 2. PUSH
            {
                strcpy(label1, " 2. PUSH ON");
                strcpy(label2, " 2. PUSH OFF");
                bool result = screen.getBinaryInputFromUser(label1, label2, button->isPatch);
                if (result != button->isSecondPushEnabled)
                {
                    button->isSecondPushEnabled = result;
                    dirty                       = true;
                }
                break;
            }
            case 7: // CC 1-8
                doPcCcEdit(button, btnNbr, CC_1_8);
                break;
            case 8: // CC 9-16
                doPcCcEdit(button, btnNbr, CC_9_16);
                break;
            case 9: // TOGL ON
            {
                strcpy(label1, "INI TOGL ON");
                strcpy(label2, "INI TOGL OFF");
                bool result = screen.getBinaryInputFromUser(label1, label2, button->isPatch);
                if (result != button->isInitialToggleStateOn)
                {
                    button->isInitialToggleStateOn = result;
                    dirty                          = true;
                }
                break;
            }
        }
    }
    return dirty;
}

bool doPcCcEdit(Button* button, uint8_t btnNbr, PcCcEnum pcCc)
{
    int btn;
    bool loop  = true;
    bool dirty = false;

    uint8_t offset = 0;
    char btnName[13];
    itoa(btnNbr, btnName, 10);
    btnName[1] = ' ';
    for (size_t i = 0; i < BUTTON_NAME_LENGTH + 1; i++)
    {
        btnName[2 + i] = button->name[i];
    }
    

    while (loop)
    {
        switch (pcCc)
        {
            case PC_1_8:
                screen.drawEdit(btnName, PC_1_EDIT_LABELS);
                break;
            case PC_9_16:
                screen.drawEdit(btnName, PC_2_EDIT_LABELS);
                offset = 8;
                break;
            case CC_1_8:
                screen.drawEdit(btnName, CC_1_EDIT_LABELS);
                break;
            case CC_9_16:
                screen.drawEdit(btnName, CC_2_EDIT_LABELS);
                offset = 8;
                break;
        }
        screen.waitForButtonReleased();
        btn = screen.getButtonPushed();
        switch (btn)
        {
            case 1: // EXIT
                loop = false;
                break;
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
                doPcCcDetailsEdit(button, btnNbr, btn - 1 + offset, pcCc);
                break;
        }
    }
    return dirty;
}

bool doPcCcDetailsEdit(Button* button, uint8_t btnNbr, uint8_t pcCcNbr, PcCcEnum pcCc)
{
    int btn;
    bool loop  = true;
    bool dirty = false;
    bool pc    = true;
    String sLabel;
    String sVal;
    uint8_t nVal;
    uint8_t newVal;
    char label[13];
    char cPcCcIndex[3];
    char btnName[13];

    itoa(pcCcNbr, cPcCcIndex, 10);
    itoa(btnNbr, btnName, 10);
    label[0]  = 'B';
    label[1]  = 'T';
    label[2]  = 'N';
    label[3]  = ' ';
    label[4]  = btnName[0];
    label[5]  = btnName[1];
    label[6]  = ' ';
    label[8]  = 'C';
    label[9]  = '#';
    label[10] = cPcCcIndex[0];
    label[11] = cPcCcIndex[1];
    label[12] = cPcCcIndex[2];
    if (label[5] == '\0')
        label[5] = ' ';
    switch (pcCc)
    {
        case PC_1_8:
        case PC_9_16:
            label[7] = 'P';
            pc = true;
            break;
        case CC_1_8:
        case CC_9_16:
            label[7] = 'C';
            
            pc = false;
            break;
    }

    while (loop)
    {
        if (pc)
            screen.drawEdit(label, PC_DETAIL_EDIT_LABELS);
        else
            screen.drawEdit(label, CC_DETAIL_EDIT_LABELS);

        screen.waitForButtonReleased();
        btn = screen.getButtonPushed();
        switch (btn)
        {
            case 1: // EXIT
                loop = false;
                break;
            case 2: {
                if (pc)
                {
                    sLabel = String("PC CH");
                    nVal   = button->pcMessages[pcCcNbr - 1].channel;
                }
                else
                {
                    sLabel = String("CC CH");
                    nVal   = button->ccMessages[pcCcNbr - 1].channel;
                }
                sVal   = String(nVal);
                Serial.println("PRE");
                newVal = screen.getNumKeyboardInputFromUser(&sLabel, &sVal, 2);
                Serial.println("POST");
                if (newVal > 16)
                    newVal = 16;
                if (nVal != newVal)
                {
                    if (pc)
                        button->pcMessages[pcCcNbr - 1].channel = newVal;
                    else
                        button->ccMessages[pcCcNbr - 1].channel = newVal;

                    dirty = true;
                }
                break;
            }
            case 3: {
                if (pc)
                {
                    sLabel = String("PC NBR");
                    nVal   = button->pcMessages[pcCcNbr - 1].valueOn;
                }
                else
                {
                    sLabel = String("CC NBR");
                    nVal   = button->ccMessages[pcCcNbr - 1].ccNumber;
                }
                sVal   = String(nVal);
                newVal = screen.getNumKeyboardInputFromUser(&sLabel, &sVal, 2);
                if (newVal > 127)
                    newVal = 127;
                if (nVal != newVal)
                {
                    if (pc)
                        button->pcMessages[pcCcNbr - 1].valueOn = newVal;
                    else
                        button->ccMessages[pcCcNbr - 1].ccNumber = newVal;

                    dirty = true;
                }
                break;
            }
            case 4:
                break;
            case 5: {
                if (pc)
                {
                    break;
                }
                else
                {
                    sLabel = String("CC MIN");
                    nVal   = button->ccMessages[pcCcNbr - 1].minValue;
                }
                sVal   = String(nVal);
                newVal = screen.getNumKeyboardInputFromUser(&sLabel, &sVal, 3);
                if (newVal > 127)
                    newVal = 127;
                if (nVal != newVal)
                {
                    button->ccMessages[pcCcNbr - 1].minValue = newVal;
                    dirty                                    = true;
                }
                break;
            }
            case 6: {
                if (pc)
                {
                    break;
                }
                else
                {
                    sLabel = String("CC MAX");
                    nVal   = button->ccMessages[pcCcNbr - 1].maxValue;
                }
                sVal   = String(nVal);
                newVal = screen.getNumKeyboardInputFromUser(&sLabel, &sVal, 3);
                if (newVal > 127)
                    newVal = 127;
                if (nVal != newVal)
                {
                    button->ccMessages[pcCcNbr - 1].maxValue = newVal;
                    dirty                                    = true;
                }
                break;
            }
            case 7:
            case 8:
            case 9:
                break;
        }
    }
    return dirty;
}

/*
 * Utility functions
 */

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

void bankNumToChar(uint8_t num, char* buf)
{
    // Precondition: buf MUST be (at least) 3 bytes long!
    itoa(num, buf, 10);
    if (num < 10)
    {
        buf[1] = buf[0];
        buf[0] = '0';
    }
    buf[2] = '\0';
}