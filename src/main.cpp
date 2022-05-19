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
void drawPlayBtns(const Bank* bank, const bool overrideSelected, const bool invertSelected = false);
void drawPlayBank(const int bnkNbr, const Bank* bank, const bool inverted);
void sendBankMidi(Bank* bank);
void sendButtonMidi(Button button, bool stateFirst);

void editMode();
void doEditUtility(Bank* bank);
bool doEditBank(Bank* bank);
bool doEditBtn(Button* button, uint8_t btnNbr);
bool doPcCcEdit(Button* button, uint8_t btnNbr, PcCcEnum pcCc);
bool doPcCcDetailsEdit(Button* button, uint8_t btnNbr, uint8_t pcCcNbr, PcCcEnum pcCc);

void loadDataStateInfo();
void saveDataStateInfo();
void bankNumToChar(uint8_t num, char* buf);
void resetToggleStates();

void setMainLabelAttributes();
void setUtilityLabelAttributes();
void setBankLabelAttributes(const Bank* bank);
void setButtonLabelAttributes(const Button* button);
void setPc1LabelAttributes(const Button* button);
void setPc2LabelAttributes(const Button* button);
void setCc1LabelAttributes(const Button* button);
void setCc2LabelAttributes(const Button* button);
void setPcDetailLabelAttributes(const PcMsg* msg);
void setCcDetailLabelAttributes(const CcMsg* msg);


EditLabelAttributes editLabelAttrib[9];

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

bool toggleStates[NUMBER_OF_BUTTONS];

enum CtrlState { PLAY,
                 PLAY_BANK_UP,
                 PLAY_BANK_DOWN,
                 EDIT };
CtrlState ctrlState = PLAY;

void setup()
{
    Serial.begin(9600);
    //    delay(1000);

    loadDataStateInfo();
    mcd = new MidiCtrlData(DATA_VERSION);

    resetToggleStates();
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
                ctrlState = EDIT;
            }
            else if (btnState->isBankUpState())
            {
                ctrlState = PLAY_BANK_UP;
            }
            else if (btnState->isBankDownState())
            {
                ctrlState = PLAY_BANK_DOWN;
            }
            break;

        case PLAY_BANK_UP:
        case PLAY_BANK_DOWN:
            ctrlState = PLAY;
            break;

        case EDIT:
            ctrlState = PLAY;
            break;
    }

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
        Bank* bank = mcd->getBank(dataState.bnk);
        Serial.println(bank->buttons[btnNbr - 1].isSecondPushEnabled);
        if (bank->buttons[btnNbr - 1].isPatch)
        {
            if (btnNbr == dataState.btn && dataState.btnStateFirst == true)
            {
                if (bank->buttons[btnNbr - 1].isSecondPushEnabled) 
                    dataState.btnStateFirst = false;
            }
            else
            {
                dataState.btnStateFirst = true;
            }

            sendButtonMidi(bank->buttons[btnNbr - 1], dataState.btnStateFirst);
            dataState.btn = btnNbr;
            saveDataStateInfo();
        }
        else
        {
            toggleStates[btnNbr - 1] = !toggleStates[btnNbr - 1];
            sendButtonMidi(bank->buttons[btnNbr - 1], toggleStates[btnNbr - 1]);
        }

        drawPlayBtns(bank, false);
    }
}

void bankChange(bool up)
{
    unsigned long prevMillis = millis();
    unsigned long curMillis;
    const unsigned long period = 500;
    int btnNbr1;
    int btnNbr2;
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
        btnNbr1 = btnState->getSingleButtonPressed();
        if (btnNbr1 > 0 && bank->buttons[btnNbr1 - 1].isPatch)  // Load selected bank
        {
            sendBankMidi(bank);

            dataState.bnk           = tmpBankNbr;
            dataState.btn           = btnNbr1;
            dataState.btnStateFirst = true;
            saveDataStateInfo();
            resetToggleStates();
            
            bBlink = true;
            while (true)
            {
                btnNbr2 = btnState->getSingleButtonPressed(true);
                if (btnNbr1 == btnNbr2)                         // The seledted button (patch) is pressed again. 
                {                                               // This allows the user to wait until the bank is loaded in the Dwarf, before sending patch related midi messages.
                    break;
                }
                curMillis = millis();
                if (curMillis - prevMillis >= period)
                {
                    bBlink = !bBlink;
                    drawPlayBtns(bank, false, bBlink);
                    prevMillis = curMillis;
                }
            }


            sendButtonMidi(bank->buttons[btnNbr1 - 1], dataState.btnStateFirst);
            
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

void drawPlayBtns(const Bank* bank, const bool overrideSelected, const bool invertSelected)
{
    byte curPatch    = dataState.btn;
    bool selected    = false;
    bool primaryFunc = true;

    for (byte i = 0; i < NUMBER_OF_BUTTONS; i++)
    {
        primaryFunc = true;
        if (!overrideSelected)
            selected = curPatch == i + 1; // Blue color patch
        if (bank->buttons[i].isPatch)
        {
            if (selected && !dataState.btnStateFirst)
                primaryFunc = false; // Red color patch
        }
        else
        { 
            if (toggleStates[i] == true)
                primaryFunc = false; // Toggle ON
        }
        screen.drawPlayButton(i + 1, bank->buttons[i].isPatch, selected, primaryFunc, bank->buttons[i].name, invertSelected);
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
                if (!button.isPatch || button.isSecondPushEnabled) // Toggle or secondPushEnabled
                    val = button.pcMessages[i].valueOff;

            usbMIDI.sendProgramChange(val, button.pcMessages[i].channel);
        }
    }

    for (uint8_t i = 0; i < NUMBER_OF_MIDI_MSG; i++)
    {
        if (button.ccMessages[i].channel > 0)
        {
            if (stateFirst)
                val = button.ccMessages[i].minValue;
            else
                val = button.ccMessages[i].maxValue;

            if (stateFirst || !button.isPatch || (button.isPatch && button.isSecondPushEnabled)) 
                // First button push - or - button is a toggle - or - second putton push with secondPushEnabled
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
        setMainLabelAttributes();
        screen.drawEdit(label, editLabelAttrib);
        screen.waitForButtonReleased();

        btn = screen.getButtonPushed();
        switch (btn)
        {
            case 0: // CALCEL
            case 1: // EXIT
                loop = false;
                break;
            case 2: // BANK
                dirty = doEditBank(bank) || dirty;
                break;
            case 3: // UTILITY
                doEditUtility(bank);
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

void doEditUtility(Bank* bank)
{
    int btn;
    bool loop  = true;

    char label1[16];
    char label2[16];
    char cNewVal[4];
    String label;
    String sVal;
    uint16_t newVal;

    while (loop)
    {
        setUtilityLabelAttributes();
        screen.drawEdit("UTILITY", editLabelAttrib);
        screen.waitForButtonReleased();

        btn = screen.getButtonPushed();
        switch (btn)
        {
            case 0: // CALCEL
            case 1: // EXIT
                loop = false;
                break;
            case 2: // COPY BANK
            {
                label = String("TO BANK");
                sVal = String(dataState.btn);
                uint8_t bankTo = screen.getNumKeyboardInputFromUser(&label, &sVal, 2);
                if (bankTo > NUMBER_OF_BANKS)
                {
                    screen.showMessage("INVALID CHOICE");
                    break;
                }

                strlcpy(label2, "   CANCEL ", sizeof(label2));
                strlcpy(label1, "BANK: ", sizeof(label1));
                itoa(dataState.bnk, cNewVal, 10);
                strlcat(label1, cNewVal, sizeof(label1));
                strlcat(label1, "->", sizeof(label1));
                itoa(bankTo, cNewVal, 10);
                strlcat(label1, cNewVal, sizeof(label1));
                if (screen.getBinaryInputFromUser(label1, label2, false))
                {
                    mcd->copyBank(dataState.bnk, bankTo);  
                }
                break;
            }
            
            case 3: // RESET BANK
            {
                strcpy(label1, " RESET BANK");
                strcpy(label2, "   CANCEL");
                if (screen.getBinaryInputFromUser(label1, label2, false))
                {
                    mcd->resetBank(dataState.bnk);  
                }
                break;
            }
            case 4: 
                break;
            case 5: // COPY BUTTON
            {
                label = String("BTN FROM");
                sVal = String(dataState.btn);
                uint8_t btnFrom = screen.getNumKeyboardInputFromUser(&label, &sVal, 1);
                if (btnFrom == 0)
                {
                    screen.showMessage("NO BUTTON CHOSEN");
                    break;
                }
                else if (btnFrom > NUMBER_OF_BUTTONS)
                {
                    screen.showMessage("INVALID CHOICE");
                    break;
                }

                label = String("TO BANK");
                sVal = String(dataState.bnk);
                uint8_t bankTo = screen.getNumKeyboardInputFromUser(&label, &sVal, 2);
                /*if (bankTo > NUMBER_OF_BANKS)
                {
                    screen.showMessage("INVALID CHOICE");
                    break;
                }*/

                label = String("TO BUTTON");
                sVal = String(dataState.btn);
                uint8_t btnTo = screen.getNumKeyboardInputFromUser(&label, &sVal, 1);
                if (btnTo == 0)
                {
                    screen.showMessage("NO BUTTON CHOSEN");
                    break;
                }
                else if (btnTo > NUMBER_OF_BANKS)
                {
                    screen.showMessage("INVALID CHOICE");
                    break;
                }
                
                strlcpy(label2, "   CANCEL ", sizeof(label2));
                strlcpy(label1, "BTN:", sizeof(label1));
                itoa(btnFrom, cNewVal, 10);
                strlcat(label1, cNewVal, sizeof(label1));
                strlcat(label1, " > ", sizeof(label1));
                itoa(bankTo, cNewVal, 10);
                strlcat(label1, cNewVal, sizeof(label1));
                strlcat(label1, " ", sizeof(label1));
                itoa(btnTo, cNewVal, 10);
                strlcat(label1, cNewVal, sizeof(label1));
                if (screen.getBinaryInputFromUser(label1, label2, false))
                {
                    mcd->copyButton(dataState.bnk, btnFrom - 1, bankTo, btnTo - 1);  
                }
                break;
            }
            case 6: // RESET BUTTON
            {
                label = String("BTN NBR");
                sVal = String("0");
                newVal = screen.getNumKeyboardInputFromUser(&label, &sVal, 1);
                Serial.println(newVal);
                if (newVal == 0)
                {
                    screen.showMessage("NO BUTTON CHOSEN");
                    break;
                }
                else if (newVal > NUMBER_OF_BUTTONS)
                {
                    screen.showMessage("INVALID CHOICE");
                    break;
                }

                strlcpy(label2, "   CANCEL", sizeof(label2));                
                itoa(newVal, cNewVal, 10);
                strlcpy(label1, "RESET BTN: ", sizeof(label1));
                strlcat(label1, cNewVal, sizeof(label1));
                if (screen.getBinaryInputFromUser(label1, label2, false))
                {
                    mcd->resetButton(dataState.bnk, newVal - 1);  
                }
                break;
            }
            case 7:
            case 8:
            case 9:
                break;
        }
    }
}

bool doEditBank(Bank* bank)
{
    int btn;
    bool loop  = true;
    bool dirty = false;

    String label;
    String sVal;
    uint8_t nVal;
    uint16_t newVal;
    char bankName[13];
    bankNumToChar(dataState.bnk, bankName);
    bankName[2] = ' ';

    while (loop)
    {
        for (size_t i = 0; i < BANK_NAME_LENGTH + 1; i++)
        {
            bankName[3 + i] = bank->name[i];
        }
        setBankLabelAttributes(bank);
        screen.drawEdit(bankName, editLabelAttrib);
        screen.waitForButtonReleased();

        btn = screen.getButtonPushed();
        switch (btn)
        {
            case 0: // CALCEL
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
        setButtonLabelAttributes(button);
        screen.drawEdit(btnName, editLabelAttrib);
        screen.waitForButtonReleased();

        btn = screen.getButtonPushed();
        switch (btn)
        {
            case 0: // CALCEL
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
                dirty = doPcCcEdit(button, btnNbr, PC_1_8) || dirty;
                break;
            case 5: // PC 9-16
                dirty = doPcCcEdit(button, btnNbr, PC_9_16);
                break;
            case 6: // 2. PUSH
            {
                strcpy(label1, " 2. PUSH ON");
                strcpy(label2, " 2. PUSH OFF");
                bool result = screen.getBinaryInputFromUser(label1, label2, button->isSecondPushEnabled);
                if (result != button->isSecondPushEnabled)
                {
                    button->isSecondPushEnabled = result;
                    dirty                       = true;
                }
                break;
            }
            case 7: // CC 1-8
                dirty = doPcCcEdit(button, btnNbr, CC_1_8) || dirty;
                break;
            case 8: // CC 9-16
                dirty = doPcCcEdit(button, btnNbr, CC_9_16);
                break;
            case 9: // TOGL ON
            {
                strcpy(label1, "INI TOGL ON");
                strcpy(label2, "INI TOGL OFF");
                bool result = screen.getBinaryInputFromUser(label1, label2, button->isInitialToggleStateOn);
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
                setPc1LabelAttributes(button);
                screen.drawEdit(btnName, editLabelAttrib);
                break;
            case PC_9_16:
                setPc2LabelAttributes(button);
                screen.drawEdit(btnName, editLabelAttrib);
                offset = 8;
                break;
            case CC_1_8:
                setCc1LabelAttributes(button);
                screen.drawEdit(btnName, editLabelAttrib);
                break;
            case CC_9_16:
                setCc2LabelAttributes(button);
                screen.drawEdit(btnName, editLabelAttrib);
                offset = 8;
                break;
        }
        screen.waitForButtonReleased();
        btn = screen.getButtonPushed();
        switch (btn)
        {
            case 0: // CALCEL
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
                dirty = doPcCcDetailsEdit(button, btnNbr, btn - 1 + offset, pcCc) || dirty;
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
    uint16_t newVal;
    char label[13];
    char cPcCcIndex[3];
    char btnName[13];
    PcMsg* pcMsg = nullptr;
    CcMsg* ccMsg = nullptr;

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
            pc       = true;
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
        {
            pcMsg = &button->pcMessages[pcCcNbr - 1];
            setPcDetailLabelAttributes(pcMsg);
        }
        else
        {
            ccMsg = &button->ccMessages[pcCcNbr - 1];
            setCcDetailLabelAttributes(ccMsg);
        }
        screen.drawEdit(label, editLabelAttrib);

        screen.waitForButtonReleased();
        btn = screen.getButtonPushed();
        switch (btn)
        {
            case 0: // CALCEL
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
                sVal = String(nVal);
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
                newVal = screen.getNumKeyboardInputFromUser(&sLabel, &sVal, 3);
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

void resetToggleStates()
{
    Bank* bank = mcd->getBank(dataState.bnk);
    for (size_t i = 0; i < NUMBER_OF_BUTTONS; i++)
    {
        toggleStates[i] = !bank->buttons[i].isPatch && bank->buttons[i].isInitialToggleStateOn; // true: Button is a toggle and initialToggleOn is true
    }
}

/*
 * Set Label Text and Attributes functions starts here.
 */
void setMainLabelAttributes()
{
    strlcpy(editLabelAttrib[0].label1, "  EXIT",9);
    strlcpy(editLabelAttrib[1].label1, "  BANK", 9);
    strlcpy(editLabelAttrib[2].label1, "UTILITY", 9);
    strlcpy(editLabelAttrib[3].label1, " BTN#4", 9);
    strlcpy(editLabelAttrib[4].label1, " BTN#5", 9);
    strlcpy(editLabelAttrib[5].label1, " BTN#6", 9);
    strlcpy(editLabelAttrib[6].label1, " BTN#1", 9);
    strlcpy(editLabelAttrib[7].label1, " BTN#2", 9);
    strlcpy(editLabelAttrib[8].label1, " BTN#3", 9);    
    
    for (size_t i = 0; i < 9; i++)
    {
        strlcpy(editLabelAttrib[i].label2, " ", LABEL_2_LENGTH);
        editLabelAttrib[i].color = 0;
    }
}

void setUtilityLabelAttributes()
{
    strlcpy(editLabelAttrib[0].label1, "  EXIT",9);
    strlcpy(editLabelAttrib[1].label1, "COPY BNK", 9);
    strlcpy(editLabelAttrib[2].label1, "RSET BNK", 9);
    strlcpy(editLabelAttrib[3].label1, " ", 9);
    strlcpy(editLabelAttrib[4].label1, "COPY BTN", 9);
    strlcpy(editLabelAttrib[5].label1, "RSET BTN", 9);
    strlcpy(editLabelAttrib[6].label1, " ", 9);
    strlcpy(editLabelAttrib[7].label1, " ", 9);
    strlcpy(editLabelAttrib[8].label1, " ", 9);    
    
    for (size_t i = 0; i < 9; i++)
    {
        strlcpy(editLabelAttrib[i].label2, " ", LABEL_2_LENGTH);
        editLabelAttrib[i].color = 0;
    }
}

void setBankLabelAttributes(const Bank* bank)
{
    strlcpy(editLabelAttrib[0].label1, "  EXIT", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[1].label1, "", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[2].label1, "", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[3].label1, "  NAME", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[4].label1, " PC CH", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[5].label1, " PC NBR", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[6].label1, "", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[7].label1, "", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[8].label1, "", LABEL_1_LENGTH);
    
    for (size_t i = 0; i < 9; i++)
    {
        strlcpy(editLabelAttrib[i].label2, " ", LABEL_2_LENGTH);
    }

    uint8_t moveSpaces = 6;
    uint8_t maxLength = 3;

    itoa(bank->pcMessage.channel, editLabelAttrib[4].label2, 10);
    itoa(bank->pcMessage.valueOn, editLabelAttrib[5].label2, 10);
    for (int i = moveSpaces + maxLength - 1; i >= 0 ; i--)
    {
        if (i > moveSpaces - 1)
        {
            editLabelAttrib[4].label2[i] = editLabelAttrib[4].label2[i - moveSpaces];
            editLabelAttrib[5].label2[i] = editLabelAttrib[5].label2[i - moveSpaces];
        }
        else
        {
            editLabelAttrib[4].label2[i] = ' ';
            editLabelAttrib[5].label2[i] = ' ';
        }  
    }
    editLabelAttrib[4].label2[moveSpaces + maxLength] = '\0';
    editLabelAttrib[5].label2[moveSpaces + maxLength] = '\0';
    
    for (size_t i = 0; i < 9; i++)
    {
        editLabelAttrib[i].color = 0;
    }
    
}

void setButtonLabelAttributes(const Button* button)
{
    strlcpy(editLabelAttrib[0].label1, "  EXIT", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[1].label1, "  NAME", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[2].label1, " PATCH", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[3].label1, " PC 1-8", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[4].label1, " PC 9-16", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[5].label1, " 2. PUSH", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[6].label1, " CC 1-8", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[7].label1, " CC 9-16", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[8].label1, " TOGL ON", LABEL_1_LENGTH);

    for (size_t i = 0; i < 9; i++)
    {
        strlcpy(editLabelAttrib[i].label2, " ", LABEL_2_LENGTH);   
    }
    
    editLabelAttrib[0].color = 0;
    editLabelAttrib[1].color = 0;

    editLabelAttrib[2].color = TFT_RED;
    if (button->isPatch)
        editLabelAttrib[2].color = TFT_GREEN;
    
    editLabelAttrib[3].color = TFT_RED;
    for (size_t i = 0; i < NUMBER_OF_MIDI_MSG / 2; i++)
        if (button->pcMessages[i].channel > 0)
        {
            editLabelAttrib[3].color = TFT_GREEN;
            break;
        }
    
    editLabelAttrib[4].color = TFT_RED;
    for (size_t i = NUMBER_OF_MIDI_MSG / 2; i < NUMBER_OF_MIDI_MSG; i++)
        if (button->pcMessages[i].channel > 0)
        {
            editLabelAttrib[4].color = TFT_GREEN;
            break;
        }
    

    editLabelAttrib[5].color = TFT_RED;
    if (button->isSecondPushEnabled)
        editLabelAttrib[5].color = TFT_GREEN;
    
    editLabelAttrib[6].color = TFT_RED;
    for (size_t i = 0; i < NUMBER_OF_MIDI_MSG / 2; i++)
        if (button->ccMessages[i].channel > 0)
        {
            editLabelAttrib[6].color = TFT_GREEN;
            break;
        }
    
    editLabelAttrib[7].color = TFT_RED;
    for (size_t i = NUMBER_OF_MIDI_MSG / 2; i < NUMBER_OF_MIDI_MSG; i++)
        if (button->ccMessages[i].channel > 0)
        {
            editLabelAttrib[7].color = TFT_GREEN;
            break;
        }
    
    editLabelAttrib[8].color = TFT_RED;
    if (button->isInitialToggleStateOn)
        editLabelAttrib[8].color = TFT_GREEN;
}

void setPc1LabelAttributes(const Button* button)
{
    strlcpy(editLabelAttrib[0].label1, "  EXIT", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[1].label1, " PC #1", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[2].label1, " PC #2", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[3].label1, " PC #3", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[4].label1, " PC #4", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[5].label1, " PC #5", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[6].label1, " PC #6", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[7].label1, " PC #7", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[8].label1, " PC #8", LABEL_1_LENGTH);

    char c[4];
    strlcpy(editLabelAttrib[0].label2, " ", LABEL_2_LENGTH);
    for (size_t i = 1; i < 9; i++)
    {
        strlcpy(editLabelAttrib[i].label2, "         ", LABEL_2_LENGTH);
        if (button->pcMessages[i - 1].channel > 0)
        {
            itoa(button->pcMessages[i - 1].channel, c, 10);
            editLabelAttrib[i].label2[3] = c[0];
            editLabelAttrib[i].label2[4] = ' ';
            if (button->pcMessages[i - 1].channel > 9)
                editLabelAttrib[i].label2[4] = c[1];
            editLabelAttrib[i].label2[5] = ' ';
            editLabelAttrib[i].label2[6] = '-';
            editLabelAttrib[i].label2[7] = ' ';
            itoa(button->pcMessages[i - 1].valueOn, c, 10);
            editLabelAttrib[i].label2[8] = c[0];
            editLabelAttrib[i].label2[9] = ' ';
            if (button->pcMessages[i - 1].valueOn > 9)
                editLabelAttrib[i].label2[9] = c[1];
            editLabelAttrib[i].label2[10] = ' ';
            if (button->pcMessages[i - 1].valueOn > 99)
                editLabelAttrib[i].label2[10] = c[2];
            editLabelAttrib[i].label2[11] = '\0';
        }
    }

    for (size_t i = 0; i < 9; i++)
    {
        editLabelAttrib[i].color = 0;
    }
}

void setPc2LabelAttributes(const Button* button)
{
    strlcpy(editLabelAttrib[0].label1, "  EXIT", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[1].label1, " PC #9", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[2].label1, " PC #10", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[3].label1, " PC #11", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[4].label1, " PC #12", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[5].label1, " PC #13", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[6].label1, " PC #14", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[7].label1, " PC #15", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[8].label1, " PC #16", LABEL_1_LENGTH);

    char c[4];
    strlcpy(editLabelAttrib[0].label2, " ", LABEL_2_LENGTH);
    for (size_t i = 1; i < 9; i++)
    {
        strlcpy(editLabelAttrib[i].label2, "       ", LABEL_2_LENGTH);
        if (button->pcMessages[i - 1 + 8].channel > 0)
        {
            itoa(button->pcMessages[i - 1 + 8].channel, c, 10);
            editLabelAttrib[i].label2[3] = c[0];
            editLabelAttrib[i].label2[4] = ' ';
            if (button->pcMessages[i - 1 + 8].channel > 9)
                editLabelAttrib[i].label2[4] = c[1];
            editLabelAttrib[i].label2[5] = ' ';
            editLabelAttrib[i].label2[6] = '-';
            editLabelAttrib[i].label2[7] = ' ';
            itoa(button->pcMessages[i - 1 + 8].valueOn, c, 10);
            editLabelAttrib[i].label2[8] = c[0];
            editLabelAttrib[i].label2[9] = ' ';
            if (button->pcMessages[i - 1 + 8].valueOn > 9)
                editLabelAttrib[i].label2[0] = c[1];
            editLabelAttrib[i].label2[10] = ' ';
            if (button->pcMessages[i - 1 + 8].valueOn > 99)
                editLabelAttrib[i].label2[10] = c[2];
            editLabelAttrib[i].label2[11] = '\0';
        }
    }

    for (size_t i = 0; i < 9; i++)
    {
        editLabelAttrib[i].color = 0;
    }
}


void setCc1LabelAttributes(const Button* button)
{
    strlcpy(editLabelAttrib[0].label1, "  EXIT", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[1].label1, " CC #1", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[2].label1, " CC #2", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[3].label1, " CC #3", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[4].label1, " CC #4", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[5].label1, " CC #5", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[6].label1, " CC #6", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[7].label1, " CC #7", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[8].label1, " CC #8", LABEL_1_LENGTH);

    char c[4];
    strlcpy(editLabelAttrib[0].label2, " ", LABEL_2_LENGTH);
    for (size_t i = 1; i < 9; i++)
    {
        strlcpy(editLabelAttrib[i].label2, " ", LABEL_2_LENGTH);
        if (button->ccMessages[i - 1].channel > 0)
        {
            itoa(button->ccMessages[i - 1].channel, c, 10);
            editLabelAttrib[i].label2[0] = c[0];
            editLabelAttrib[i].label2[1] = ' ';
            if (button->ccMessages[i - 1].channel > 9)
                editLabelAttrib[i].label2[1] = c[1];
            editLabelAttrib[i].label2[2] = ' '; // Space
            itoa(button->ccMessages[i - 1].ccNumber, c, 10);
            editLabelAttrib[i].label2[3] = c[0];
            editLabelAttrib[i].label2[4] = ' ';
            if (button->ccMessages[i - 1].ccNumber > 9)
                editLabelAttrib[i].label2[4] = c[1];
            editLabelAttrib[i].label2[5] = ' ';
            if (button->ccMessages[i - 1].ccNumber > 99)
                editLabelAttrib[i].label2[5] = c[2];
            editLabelAttrib[i].label2[6] = ' '; // Space
            itoa(button->ccMessages[i - 1].minValue, c, 10);
            editLabelAttrib[i].label2[7] = c[0];
            editLabelAttrib[i].label2[8] = ' ';
            if (button->ccMessages[i - 1].minValue > 9)
                editLabelAttrib[i].label2[8] = c[1];
            editLabelAttrib[i].label2[9] = ' ';
            if (button->ccMessages[i - 1].minValue > 99)
                editLabelAttrib[i].label2[9] = c[2];
            editLabelAttrib[i].label2[10] = ' '; // Space
            itoa(button->ccMessages[i - 1].maxValue, c, 10);
            editLabelAttrib[i].label2[11] = c[0];
            editLabelAttrib[i].label2[12] = ' ';
            if (button->ccMessages[i - 1].maxValue > 9)
                editLabelAttrib[i].label2[12] = c[1];
            editLabelAttrib[i].label2[13] = ' ';
            if (button->ccMessages[i - 1].maxValue > 99)
                editLabelAttrib[i].label2[13] = c[2];
            
            editLabelAttrib[i].label2[14] = '\0';
        }
    }

    for (size_t i = 0; i < 9; i++)
    {
        editLabelAttrib[i].color = 0;
    }
}

void setCc2LabelAttributes(const Button* button)
{
    strlcpy(editLabelAttrib[0].label1, "  EXIT", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[1].label1, " CC #9", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[2].label1, " CC #10", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[3].label1, " CC #11", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[4].label1, " CC #12", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[5].label1, " CC #13", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[6].label1, " CC #14", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[7].label1, " CC #15", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[8].label1, " CC #16", LABEL_1_LENGTH);

    char c[4];
    strlcpy(editLabelAttrib[0].label2, " ", LABEL_2_LENGTH);
    for (size_t i = 1; i < 9; i++)
    {
        strlcpy(editLabelAttrib[i].label2, " ", LABEL_2_LENGTH);
        if (button->ccMessages[i - 1].channel > 0)
        {
            itoa(button->ccMessages[i - 1 + 8].channel, c, 10);
            editLabelAttrib[i].label2[0] = c[0];
            editLabelAttrib[i].label2[1] = ' ';
            if (button->ccMessages[i - 1 + 8].channel > 9)
                editLabelAttrib[i].label2[1] = c[1];
            editLabelAttrib[i].label2[2] = ' '; // Space
            itoa(button->ccMessages[i - 1 + 8].ccNumber, c, 10);
            editLabelAttrib[i].label2[3] = c[0];
            editLabelAttrib[i].label2[4] = ' ';
            if (button->ccMessages[i - 1 + 8].ccNumber > 9)
                editLabelAttrib[i].label2[4] = c[1];
            editLabelAttrib[i].label2[5] = ' ';
            if (button->ccMessages[i - 1 + 8].ccNumber > 99)
                editLabelAttrib[i].label2[5] = c[2];
            editLabelAttrib[i].label2[6] = ' '; // Space
            itoa(button->ccMessages[i - 1 + 8].minValue, c, 10);
            editLabelAttrib[i].label2[7] = c[0];
            editLabelAttrib[i].label2[8] = ' ';
            if (button->ccMessages[i - 1 + 8].minValue > 9)
                editLabelAttrib[i].label2[8] = c[1];
            editLabelAttrib[i].label2[9] = ' ';
            if (button->ccMessages[i - 1 + 8].minValue > 99)
                editLabelAttrib[i].label2[9] = c[2];
            editLabelAttrib[i].label2[10] = ' '; // Space
            itoa(button->ccMessages[i - 1 + 8].maxValue, c, 10);
            editLabelAttrib[i].label2[11] = c[0];
            editLabelAttrib[i].label2[12] = ' ';
            if (button->ccMessages[i - 1 + 8].maxValue > 9)
                editLabelAttrib[i].label2[12] = c[1];
            editLabelAttrib[i].label2[13] = ' ';
            if (button->ccMessages[i - 1 + 8].maxValue > 99)
                editLabelAttrib[i].label2[13] = c[2];
            
            editLabelAttrib[i].label2[14] = '\0';
        }
    }

    for (size_t i = 0; i < 9; i++)
    {
        editLabelAttrib[i].color = 0;
    }
}

void setPcDetailLabelAttributes(const PcMsg* msg)
{
    strlcpy(editLabelAttrib[0].label1, "  EXIT", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[1].label1, " PC CH", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[2].label1, " PC NBR", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[3].label1, " ", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[4].label1, " ", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[5].label1, " ", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[6].label1, " ", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[7].label1, " ", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[8].label1, " ", LABEL_1_LENGTH);

    for (size_t i = 0; i < 9; i++)
        strlcpy(editLabelAttrib[i].label2, "          ", LABEL_2_LENGTH);

    char c[4];
    if (msg->channel > 0)
    {
        itoa(msg->channel, c, 10);
        editLabelAttrib[1].label2[5] = c[0];
        editLabelAttrib[1].label2[6] = c[1];
        editLabelAttrib[1].label2[7] = c[2];
    }
    if (msg->valueOn > 0)
    {
        itoa(msg->valueOn, c, 10);
        editLabelAttrib[2].label2[5] = c[0];
        editLabelAttrib[2].label2[6] = c[1];
        editLabelAttrib[2].label2[7] = c[2];
        editLabelAttrib[2].label2[8] = c[3];
    }    
    
    for (size_t i = 0; i < 9; i++)
    {
        editLabelAttrib[i].color = 0;
    }
}

void setCcDetailLabelAttributes(const CcMsg* msg)
{
    strlcpy(editLabelAttrib[0].label1, "  EXIT", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[1].label1, " CC CH", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[2].label1, " CC NBR", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[3].label1, " ", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[4].label1, "MIN VAL", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[5].label1, "MAX VAL", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[6].label1, " ", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[7].label1, " ", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[8].label1, " ", LABEL_1_LENGTH);

    for (size_t i = 0; i < 9; i++)
        strlcpy(editLabelAttrib[i].label2, "          ", LABEL_2_LENGTH);

    char c[4];
    if (msg->channel > 0)
    {
        itoa(msg->channel, c, 10);
        editLabelAttrib[1].label2[5] = c[0];
        editLabelAttrib[1].label2[6] = c[1];
        editLabelAttrib[1].label2[7] = c[2];
    
        itoa(msg->ccNumber, c, 10);
        editLabelAttrib[2].label2[5] = c[0];
        editLabelAttrib[2].label2[6] = c[1];
        editLabelAttrib[2].label2[7] = c[2];
        editLabelAttrib[2].label2[8] = c[3];
    
        itoa(msg->minValue, c, 10);
        editLabelAttrib[4].label2[5] = c[0];
        editLabelAttrib[4].label2[6] = c[1];
        editLabelAttrib[4].label2[7] = c[2];
        editLabelAttrib[4].label2[8] = c[3];
    
        itoa(msg->maxValue, c, 10);
        editLabelAttrib[5].label2[5] = c[0];
        editLabelAttrib[5].label2[6] = c[1];
        editLabelAttrib[5].label2[7] = c[2];
        editLabelAttrib[5].label2[8] = c[3];
    }    

    for (size_t i = 0; i < 9; i++)
    {
        editLabelAttrib[i].color = 0;
    }
}