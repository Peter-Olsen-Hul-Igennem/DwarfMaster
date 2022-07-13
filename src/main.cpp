#include "ButtonState.h"
#include "ExpressionHandler.h"
#include "GlobalConst.h"
#include "MidiCtrlData.h"
#include "MidiTransmitter.h"
#include "Screen.h"
#include "Touch.h"
#include <Arduino.h>
#include <EEPROM.h>

enum CtrlState { PLAY,
                 PLAY_BANK_UP,
                 PLAY_BANK_DOWN,
                 EDIT_BANK,
                 EDIT_BTN };
CtrlState ctrlState = PLAY;

enum BtnPushState { VERY_FIRST_BTN_PUSH,
                    FIRST_BTN_PUSH,
                    SECOND_BTN_PUSH };

enum PcCcEnum { PC_1_8,
                PC_9_16,
                CC_1_8,
                CC_9_16 };

const uint16_t DATA_STATE_EEPROM_ADR = 0;
const uint8_t EXP_PIN                = 19;

void setup();
void loop();

void playMode();
void bankChange(bool up);
void drawPlayScreen();
void drawPlayBtns(const bool overrideSelected, const bool invertSelected = false);
void drawPlayBank(const int bnkNbr, const bool inverted);
void sendButtonMidi(Button button, BtnPushState btnState);

void editMode(uint8_t editBtnNbr);
void doEditUtility();
bool doEditBank();
bool doEditBtn(uint8_t btnNbr);
bool doPcCcEdit(Button* button, uint8_t btnNbr, PcCcEnum pcCc);
bool doPcCcDetailsEdit(Button* button, uint8_t btnNbr, uint8_t pcCcNbr, PcCcEnum pcCc);

void loadDataStateInfo();
void saveDataStateInfo();
void bankNumToChar(uint8_t num, char* buf);
void initHousekeepingVariables();
void togglePushBtnState(uint8_t btnNbr);

void setMainLabelAttributes();
void setUtilityLabelAttributes();
void setBankLabelAttributes();
void setButtonLabelAttributes(const Button* button);
void setPc1LabelAttributes(const Button* button);
void setPc2LabelAttributes(const Button* button);
void setCc1LabelAttributes(const Button* button);
void setCc2LabelAttributes(const Button* button);
void setPcDetailLabelAttributes(const PcMsg* msg);
void setCcDetailLabelAttributes(const CcMsg* msg);

EditLabelAttributes editLabelAttrib[9];

Screen screen; // TODO pointer
MidiCtrlData* mcd;
Bank* bank;
ButtonState* btnState = btnState->getInstance();
MidiTransmitter* midiTransmitter;
ExpressionHandler* expressionHandler;

struct DataState
{
    uint8_t bnk;
    uint8_t btn;
    BtnPushState btnPushState = VERY_FIRST_BTN_PUSH;
    uint16_t expMin;
    uint16_t expMax;
};
DataState dataState;

bool toggleStates[NUMBER_OF_BUTTONS]; // Keeping track of the states of the toggle switches (buttons not set up to be patches). True is ON.

void setup()
{
    Serial.begin(9600);
    //    delay(1000);

    loadDataStateInfo();
    mcd  = new MidiCtrlData();
    bank = mcd->getBank(dataState.bnk);

    expressionHandler = new ExpressionHandler(EXP_PIN, dataState.expMin, dataState.expMax);
    midiTransmitter   = new MidiTransmitter(expressionHandler);

    initHousekeepingVariables();

    screen.showWaitScreen();
    screen.waitForButtonReleased();

    sendButtonMidi(bank->buttons[dataState.btn - 1], dataState.btnPushState);

    drawPlayScreen();
}

void loop()
{
    uint8_t playBtnPressed = 0;

    // switch that sets up state
    switch (ctrlState)
    {
        case PLAY:
            if (btnState->isBankUpState())
            {
                ctrlState = PLAY_BANK_UP;
            }
            else if (btnState->isBankDownState())
            {
                ctrlState = PLAY_BANK_DOWN;
            }
            else if (screen.playBankPressed())
            {
                ctrlState = EDIT_BANK;
            }
            else
            {
                playBtnPressed = screen.playBtnPressed();
                if (playBtnPressed > 0)
                {
                    ctrlState = EDIT_BTN;
                }
            }

            break;

        case PLAY_BANK_UP:
        case PLAY_BANK_DOWN:
            ctrlState = PLAY;
            break;

        case EDIT_BANK:
        case EDIT_BTN:
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
        case EDIT_BANK:
            editMode(0);
            break;
        case EDIT_BTN:
            editMode(playBtnPressed);
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
    if (btnNbr > 0) // A footswitch was pushed
    {
        if (bank->buttons[btnNbr - 1].isPatch)
        {
            if (btnNbr == dataState.btn)
                togglePushBtnState(btnNbr - 1);
            else
                dataState.btnPushState = VERY_FIRST_BTN_PUSH;
            
            sendButtonMidi(bank->buttons[btnNbr - 1], dataState.btnPushState);
            dataState.btn = btnNbr;
            saveDataStateInfo();
        }
        else
        {
            toggleStates[btnNbr - 1] = !toggleStates[btnNbr - 1];
            if (toggleStates[btnNbr - 1])
                sendButtonMidi(bank->buttons[btnNbr - 1], FIRST_BTN_PUSH);
            else
                sendButtonMidi(bank->buttons[btnNbr - 1], SECOND_BTN_PUSH);
        }

        drawPlayBtns(false);
    }
    else
    {
        for (uint8_t i = 0; i < NUMBER_OF_BUTTONS; i++)
        {
            if (!bank->buttons[i].isLatching && btnState->isButtonJustReleased(i + 1))
            {
                if (bank->buttons[i].isPatch)
                    togglePushBtnState(i);
                else
                    toggleStates[i] = !toggleStates[i];
                
                sendButtonMidi(bank->buttons[i], SECOND_BTN_PUSH);
                drawPlayBtns(false);
            }
        }
    }

    // Send Expressionpedal midi
    midiTransmitter->sendExpressionMessages(bank, toggleStates, dataState.btn);
}

void bankChange(bool up)
{
    unsigned long prevMillis = millis();
    unsigned long curMillis;
    const unsigned long period = 500;
    int btnNbr1;
    int btnNbr2;
    bool bBlink     = false;
    bool first      = true;
    byte tmpBankNbr = dataState.bnk;

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
            drawPlayBank(tmpBankNbr, bBlink);
            drawPlayBtns(true);
            prevMillis = curMillis;
        }
        btnNbr1 = btnState->getSingleButtonPressed();
        if (btnNbr1 > 0 && bank->buttons[btnNbr1 - 1].isPatch) // Load selected bank
        {
            midiTransmitter->sendPcMsg(bank->pcMessage, true); // Send Bank Midi

            dataState.bnk          = tmpBankNbr;
            dataState.btn          = btnNbr1;
            dataState.btnPushState = VERY_FIRST_BTN_PUSH;
            saveDataStateInfo();
            initHousekeepingVariables();

            bBlink = true;
            while (true)
            {
                btnNbr2 = btnState->getSingleButtonPressed(true);
                if (btnNbr1 == btnNbr2) // The seledted button (patch) is pressed again.
                {                       // This allows the user to wait until the bank is loaded in the Dwarf, before sending patch related midi messages.
                    break;
                }
                curMillis = millis();
                if (curMillis - prevMillis >= period)
                {
                    bBlink = !bBlink;
                    drawPlayBtns(false, bBlink);
                    prevMillis = curMillis;
                }
                if (usbMIDI.read()) // Recieving any midi data from the Dwarf will signal that the bank is loaded. Tip: The InfamousMindi will send a midi msg on loading! 
                    break;
            }

            sendButtonMidi(bank->buttons[btnNbr1 - 1], dataState.btnPushState);

            drawPlayScreen();

            break;
        }
    }
}

void drawPlayScreen()
{
    screen.blankScreen();
    drawPlayBtns(false);
    drawPlayBank(dataState.bnk, false);
}

void drawPlayBtns(const bool overrideSelected, const bool invertSelected)
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
            if (selected && dataState.btnPushState == SECOND_BTN_PUSH)
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

void drawPlayBank(const int bnkNbr, const bool inverted)
{
    char bankNumber[3];
    bankNumToChar(bnkNbr, bankNumber);
    screen.drawPlayBank(bankNumber, bank->name, inverted);
}

void sendButtonMidi(Button button, BtnPushState btnState)
{
    for (uint8_t i = 0; i < NUMBER_OF_MIDI_MSG; i++)
    {
        switch (btnState)
        {
            case VERY_FIRST_BTN_PUSH:
                midiTransmitter->sendPcMsg(button.pcMessages[i], true);
                break;

            case FIRST_BTN_PUSH:
                if (button.pcMessages[i].valueOn != button.pcMessages[i].valueOff) // If the On and Off values are the same, the PC is not (re)sent.
                    midiTransmitter->sendPcMsg(button.pcMessages[i], true);
                break;

            case SECOND_BTN_PUSH:
                if (!button.isPatch || button.isSecondPushEnabled)                     // Toggle or secondPushEnabled
                    if (button.pcMessages[i].valueOn != button.pcMessages[i].valueOff) // If the On and Off values are the same, the PC is not (re)sent.
                        midiTransmitter->sendPcMsg(button.pcMessages[i], false);
                break;
        }
    }

    for (uint8_t i = 0; i < NUMBER_OF_MIDI_MSG; i++)
    {
        switch (btnState)
        {
            case VERY_FIRST_BTN_PUSH:
            case FIRST_BTN_PUSH:
                midiTransmitter->sendCcMsg(button.ccMessages[i], true, button.ccMessages[i].useMaxValAsCcNbr);
                break;

            case SECOND_BTN_PUSH:
                if (!button.isPatch || button.isSecondPushEnabled)
                {
                    midiTransmitter->sendCcMsg(button.ccMessages[i], false, button.ccMessages[i].useMaxValAsCcNbr);
                }
                break;
        }
    }
}

/*
 * EDIT functions starts here
 */
void editMode(uint8_t editBtnNbr)
{
    bool dirty = false;
    char label[13];
    bankNumToChar(dataState.bnk, label);
    label[2] = ' ';

    int btn;
    bool loop = true;

    if (editBtnNbr > 0) // A field representing a button on the screen was pressed
    {
        loop  = false;
        dirty = doEditBtn(editBtnNbr);
    }

    while (loop)
    {
        strlcpy(label + 3, bank->name, 13);
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
                dirty = doEditBank() || dirty;
                break;
            case 3: // UTILITY
                doEditUtility();
                break;
            case 4: // BTN #4
            case 5: // BTN #5
            case 6: // BTN #6
            {
                dirty = doEditBtn(btn) || dirty;
                break;
            }
            case 7: // BTN #1
            case 8: // BTN #2
            case 9: // BTN #3
            {
                dirty = doEditBtn(btn - 6) || dirty;
                break;
            }
        }
    }

    if (dirty)
        mcd->saveBank(dataState.bnk);

    drawPlayScreen();
    screen.waitForButtonReleased();
}

void doEditUtility()
{
    int btn;
    bool loop = true;

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
                label          = String("TO BANK");
                sVal           = String(dataState.btn);
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
            case 4: // EXP CALIBRATION
            {
                screen.showMessage("MOVE PEDAL TO THE <MIN> POSITION AND PRESS THE SCREEN");
                dataState.expMin = expressionHandler->readCalibrationValue(false);
                screen.showMessage("MOVE PEDAL TO THE <MAX> POSITION AND PRESS THE SCREEN");
                dataState.expMax = expressionHandler->readCalibrationValue(true);

                char msg[85];
                char val[5];
                strlcpy(msg, "MIN: ", 85);
                itoa(dataState.expMin, val, 10);
                strlcpy(msg + strlen(msg), val, 85);
                strlcpy(msg + strlen(msg), "         MAX: ", 85);
                itoa(dataState.expMax, val, 10);
                strlcpy(msg + strlen(msg), val, 85);
                screen.showMessage(msg);

                saveDataStateInfo();
                break;
            }
            case 5: // COPY BUTTON
            {
                label           = String("BTN FROM");
                sVal            = String(dataState.btn);
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

                label          = String("TO BANK");
                sVal           = String(dataState.bnk);
                uint8_t bankTo = screen.getNumKeyboardInputFromUser(&label, &sVal, 2);
                // No need to check if bankTo > 99, because getNumKeyboardInputFromUser(&label, &sVal, 2) only allows 2 digits

                label         = String("TO BUTTON");
                sVal          = String(dataState.btn);
                uint8_t btnTo = screen.getNumKeyboardInputFromUser(&label, &sVal, 1);
                if (btnTo == 0)
                {
                    screen.showMessage("NO BUTTON CHOSEN");
                    break;
                }
                else if (btnTo > NUMBER_OF_BUTTONS)
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
                label  = String("BTN NBR");
                sVal   = String("0");
                newVal = screen.getNumKeyboardInputFromUser(&label, &sVal, 1);
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
            case 7: {
                label      = String("CHANNEL");
                sVal       = String("1");
                uint8_t ch = screen.getNumKeyboardInputFromUser(&label, &sVal, 2);
                if (ch == 0)
                {
                    screen.showMessage("NO CHANNEL CHOSEN");
                    break;
                }
                else if (ch > 16)
                {
                    screen.showMessage("INVALID CHOICE");
                    break;
                }

                label       = String("CC NBR");
                sVal        = String("0");
                uint16_t cc = screen.getNumKeyboardInputFromUser(&label, &sVal, 3);
                if (cc > 127)
                {
                    screen.showMessage("INVALID CHOICE");
                    break;
                }

                strlcpy(label2, "   CANCEL", sizeof(label2));
                strlcpy(label1, "CH:", sizeof(label1));
                itoa(ch, cNewVal, 10);
                strlcat(label1, cNewVal, sizeof(label1));
                strlcat(label1, " CC:", sizeof(label1));
                itoa(cc, cNewVal, 10);
                strlcat(label1, cNewVal, sizeof(label1));
                if (screen.getBinaryInputFromUser(label1, label2, false))
                {
                    usbMIDI.sendControlChange(cc, 127, ch);
                }
                break;
            }
            case 8:
            case 9:
                break;
        }
    }
}

bool doEditBank()
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
        strlcpy(bankName + 3, bank->name, 13);
        setBankLabelAttributes();
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

bool doEditBtn(const uint8_t btnNbr)
{
    int btn;
    bool loop      = true;
    bool dirty     = false;
    Button* button = &bank->buttons[btnNbr - 1];
    String label;
    String sVal;
    char btnName[13];
    itoa(btnNbr, btnName, 10);
    btnName[1] = ' ';

    while (loop)
    {
        strlcpy(btnName + 2, button->name, 13);
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
                button->isPatch = !button->isPatch;
                dirty           = true;
                break;
            }
            case 4: // PC 1-8
                dirty = doPcCcEdit(button, btnNbr, PC_1_8) || dirty;
                break;
            case 5: // PC 9-16
                dirty = doPcCcEdit(button, btnNbr, PC_9_16) || dirty;
                break;
            case 6: // 2. PUSH + Initial togglestate
            {
                if (button->isPatch)
                    button->isSecondPushEnabled = !button->isSecondPushEnabled;
                else
                    button->isInitialToggleStateOn = !button->isInitialToggleStateOn;
                dirty = true;
                break;
            }
            case 7: // CC 1-8
                dirty = doPcCcEdit(button, btnNbr, CC_1_8) || dirty;
                break;
            case 8: // CC 9-16
                dirty = doPcCcEdit(button, btnNbr, CC_9_16) || dirty;
                break;
            case 9: // LATCH
            {
                if (button->isPatch)
                {
                    if (button->isSecondPushEnabled)
                    {
                        button->isLatching = !button->isLatching;
                        dirty              = true;
                    }
                }
                else
                {
                    button->isLatching = !button->isLatching;
                    dirty              = true;
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
    strlcpy(btnName + 2, button->name, 13);

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
    PcMsg* pcMsg = nullptr;
    CcMsg* ccMsg = nullptr;

    strlcpy(label, "BTN#", 13);
    itoa(btnNbr, label + 4, 10);
    switch (pcCc)
    {
        case PC_1_8:
        case PC_9_16:
            strlcpy(label + 5, "  PC#", 13);
            pc = true;
            break;
        case CC_1_8:
        case CC_9_16:
            strlcpy(label + 5, "  CC#", 13);
            pc = false;
            break;
    }
    itoa(pcCcNbr, label + 10, 10);

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
                loop = false;
                break;
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
                newVal = screen.getNumKeyboardInputFromUser(&sLabel, &sVal, 2);
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
                    break;
                }

                sLabel = String("CC NBR");
                nVal   = button->ccMessages[pcCcNbr - 1].ccNumber;
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
                    sLabel = String("NBR ON");
                    nVal   = button->pcMessages[pcCcNbr - 1].valueOn;
                    sVal   = String(nVal);
                    newVal = screen.getNumKeyboardInputFromUser(&sLabel, &sVal, 3);
                    if (newVal > 127)
                        newVal = 127;
                    if (nVal != newVal)
                    {
                        button->pcMessages[pcCcNbr - 1].valueOn = newVal;
                        dirty                                   = true;
                    }
                    break;
                }

                sLabel = String("CC MIN");
                nVal   = button->ccMessages[pcCcNbr - 1].minValue;
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
                    sLabel = String("NBR OFF");
                    nVal   = button->pcMessages[pcCcNbr - 1].valueOff;
                    sVal   = String(nVal);
                    newVal = screen.getNumKeyboardInputFromUser(&sLabel, &sVal, 3);
                    if (newVal > 127)
                        newVal = 127;
                    if (nVal != newVal)
                    {
                        button->pcMessages[pcCcNbr - 1].valueOff = newVal;
                        dirty                                    = true;
                    }
                    break;
                }

                sLabel = String("CC MAX");
                nVal   = button->ccMessages[pcCcNbr - 1].maxValue;
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
            case 7: {
                if (pc)
                {
                    break;
                }
                button->ccMessages[pcCcNbr - 1].ctrlByExp = !button->ccMessages[pcCcNbr - 1].ctrlByExp;
                dirty                                     = true;
                break;
            }
            case 8:
                break;
            case 9: {
                if (pc)
                {
                    break;
                }
                button->ccMessages[pcCcNbr - 1].useMaxValAsCcNbr = !button->ccMessages[pcCcNbr - 1].useMaxValAsCcNbr;
                dirty                                            = true;
                break;
            }
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
        dataState.btn          = 1;
        dataState.btnPushState = VERY_FIRST_BTN_PUSH;
    }
    if (dataState.expMax < 0 || dataState.expMax > 1023)
        dataState.expMax = 1023;
    if (dataState.expMin < 0 || dataState.expMin > 1023)
        dataState.expMin = 0;
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

void initHousekeepingVariables()
{
    for (size_t i = 0; i < NUMBER_OF_BUTTONS; i++)
    {
        toggleStates[i] = !bank->buttons[i].isPatch && bank->buttons[i].isInitialToggleStateOn; // true: Button is a toggle and initialToggleOn is true
    }
}

void togglePushBtnState(uint8_t btnNbr)
{
    switch (dataState.btnPushState)
    {
        case VERY_FIRST_BTN_PUSH:
        case FIRST_BTN_PUSH:
            if (bank->buttons[btnNbr].isSecondPushEnabled)
                dataState.btnPushState = SECOND_BTN_PUSH;
            break;

        case SECOND_BTN_PUSH:
            dataState.btnPushState = FIRST_BTN_PUSH;
            break;
    }
}

/*
 * Set Label Text and Attributes functions starts here.
 */
void setMainLabelAttributes()
{
    strlcpy(editLabelAttrib[0].label1, "EXIT", 9);
    strlcpy(editLabelAttrib[1].label1, "BANK", 9);
    strlcpy(editLabelAttrib[2].label1, "UTILITY", 9);
    strlcpy(editLabelAttrib[3].label1, "BTN#4", 9);
    strlcpy(editLabelAttrib[4].label1, "BTN#5", 9);
    strlcpy(editLabelAttrib[5].label1, "BTN#6", 9);
    strlcpy(editLabelAttrib[6].label1, "BTN#1", 9);
    strlcpy(editLabelAttrib[7].label1, "BTN#2", 9);
    strlcpy(editLabelAttrib[8].label1, "BTN#3", 9);

    for (size_t i = 0; i < 9; i++)
    {
        strlcpy(editLabelAttrib[i].label2, " ", LABEL_2_LENGTH);
        editLabelAttrib[i].color = 0;
    }
}

void setUtilityLabelAttributes()
{
    strlcpy(editLabelAttrib[0].label1, "EXIT", 9);
    strlcpy(editLabelAttrib[1].label1, "COPY BNK", 9);
    strlcpy(editLabelAttrib[2].label1, "RSET BNK", 9);
    strlcpy(editLabelAttrib[3].label1, "EXP", 9);
    strlcpy(editLabelAttrib[4].label1, "COPY BTN", 9);
    strlcpy(editLabelAttrib[5].label1, "RSET BTN", 9);
    strlcpy(editLabelAttrib[6].label1, "SEND CC", 9);
    strlcpy(editLabelAttrib[7].label1, " ", 9);
    strlcpy(editLabelAttrib[8].label1, " ", 9);

    for (size_t i = 0; i < 9; i++)
    {
        strlcpy(editLabelAttrib[i].label2, " ", LABEL_2_LENGTH);
        editLabelAttrib[i].color = 0;
    }
    strlcpy(editLabelAttrib[3].label2, "CALIBRATION", LABEL_2_LENGTH);
}

void setBankLabelAttributes()
{
    strlcpy(editLabelAttrib[0].label1, "EXIT", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[1].label1, "", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[2].label1, "", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[3].label1, "NAME", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[4].label1, "PC CH", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[5].label1, "PC NBR", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[6].label1, "", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[7].label1, "", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[8].label1, "", LABEL_1_LENGTH);

    for (size_t i = 0; i < 9; i++)
    {
        strlcpy(editLabelAttrib[i].label2, " ", LABEL_2_LENGTH);
    }
    itoa(bank->pcMessage.channel, editLabelAttrib[4].label2, 10);
    itoa(bank->pcMessage.valueOn, editLabelAttrib[5].label2, 10);

    for (size_t i = 0; i < 9; i++)
    {
        editLabelAttrib[i].color = 0;
    }
}

void setButtonLabelAttributes(const Button* button)
{
    strlcpy(editLabelAttrib[0].label1, "EXIT", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[1].label1, "NAME", LABEL_1_LENGTH);

    strlcpy(editLabelAttrib[3].label1, "PC 1-8", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[4].label1, "PC 9-16", LABEL_1_LENGTH);

    strlcpy(editLabelAttrib[6].label1, "CC 1-8", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[7].label1, "CC 9-16", LABEL_1_LENGTH);

    if (button->isPatch)
    {
        strlcpy(editLabelAttrib[2].label1, "PATCH", LABEL_1_LENGTH);
        if (button->isSecondPushEnabled)
        {
            strlcpy(editLabelAttrib[5].label1, "2. ON", LABEL_1_LENGTH);
            if (button->isLatching)
                strlcpy(editLabelAttrib[8].label1, "LATCH", LABEL_1_LENGTH);
            else
                strlcpy(editLabelAttrib[8].label1, "UNLATCH", LABEL_1_LENGTH);
        }
        else
        {
            strlcpy(editLabelAttrib[5].label1, "2. OFF", LABEL_1_LENGTH);
            strlcpy(editLabelAttrib[8].label1, " ", LABEL_1_LENGTH);
        }
    }
    else
    {
        strlcpy(editLabelAttrib[2].label1, "TOGGLE", LABEL_1_LENGTH);
        if (button->isInitialToggleStateOn)
            strlcpy(editLabelAttrib[5].label1, "INIT ON", LABEL_1_LENGTH);
        else
            strlcpy(editLabelAttrib[5].label1, "INIT OFF", LABEL_1_LENGTH);
        if (button->isLatching)
            strlcpy(editLabelAttrib[8].label1, "LATCH", LABEL_1_LENGTH);
        else
            strlcpy(editLabelAttrib[8].label1, "UNLATCH", LABEL_1_LENGTH);
    }

    for (size_t i = 0; i < 9; i++)
    {
        strlcpy(editLabelAttrib[i].label2, " ", LABEL_2_LENGTH);
    }

    editLabelAttrib[0].color = 0;
    editLabelAttrib[1].color = 0;
    editLabelAttrib[2].color = 0;

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

    editLabelAttrib[5].color = 0;

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
    editLabelAttrib[8].color = 0;
}

void setPc1LabelAttributes(const Button* button)
{
    strlcpy(editLabelAttrib[0].label1, "EXIT", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[1].label1, "PC #1", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[2].label1, "PC #2", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[3].label1, "PC #3", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[4].label1, "PC #4", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[5].label1, "PC #5", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[6].label1, "PC #6", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[7].label1, "PC #7", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[8].label1, "PC #8", LABEL_1_LENGTH);

    strlcpy(editLabelAttrib[0].label2, " ", LABEL_2_LENGTH);
    for (size_t i = 1; i < 9; i++)
    {
        strlcpy(editLabelAttrib[i].label2, "         ", LABEL_2_LENGTH);
        if (button->pcMessages[i - 1].channel > 0)
        {
            itoa(button->pcMessages[i - 1].channel, editLabelAttrib[i].label2, 10);
            strlcpy(editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), " ", LABEL_2_LENGTH);
            itoa(button->pcMessages[i - 1].valueOn, editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), 10);
            strlcpy(editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), " ", LABEL_2_LENGTH);
            itoa(button->pcMessages[i - 1].valueOff, editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), 10);
        }
    }

    for (size_t i = 0; i < 9; i++)
    {
        editLabelAttrib[i].color = 0;
    }
}

void setPc2LabelAttributes(const Button* button)
{
    strlcpy(editLabelAttrib[0].label1, "EXIT", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[1].label1, "PC #9", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[2].label1, "PC #10", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[3].label1, "PC #11", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[4].label1, "PC #12", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[5].label1, "PC #13", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[6].label1, "PC #14", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[7].label1, "PC #15", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[8].label1, "PC #16", LABEL_1_LENGTH);

    strlcpy(editLabelAttrib[0].label2, " ", LABEL_2_LENGTH);
    for (size_t i = 1; i < 9; i++)
    {
        strlcpy(editLabelAttrib[i].label2, "       ", LABEL_2_LENGTH);
        if (button->pcMessages[i - 1 + 8].channel > 0)
        {
            itoa(button->pcMessages[i - 1 + 8].channel, editLabelAttrib[i].label2, 10);
            strlcpy(editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), " ", LABEL_2_LENGTH);
            itoa(button->pcMessages[i - 1 + 8].valueOn, editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), 10);
            strlcpy(editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), " ", LABEL_2_LENGTH);
            itoa(button->pcMessages[i - 1 + 8].valueOff, editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), 10);
        }
    }

    for (size_t i = 0; i < 9; i++)
    {
        editLabelAttrib[i].color = 0;
    }
}

void setCc1LabelAttributes(const Button* button)
{
    strlcpy(editLabelAttrib[0].label1, "EXIT", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[1].label1, "CC #1", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[2].label1, "CC #2", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[3].label1, "CC #3", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[4].label1, "CC #4", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[5].label1, "CC #5", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[6].label1, "CC #6", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[7].label1, "CC #7", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[8].label1, "CC #8", LABEL_1_LENGTH);

    strlcpy(editLabelAttrib[0].label2, " ", LABEL_2_LENGTH);
    for (size_t i = 1; i < 9; i++)
    {
        strlcpy(editLabelAttrib[i].label2, "", LABEL_2_LENGTH);
        if (button->ccMessages[i - 1].channel > 0)
        {
            itoa(button->ccMessages[i - 1].channel, editLabelAttrib[i].label2, 10);
            strlcpy(editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), " ", LABEL_2_LENGTH);
            itoa(button->ccMessages[i - 1].ccNumber, editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), 10);
            strlcpy(editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), " ", LABEL_2_LENGTH);
            itoa(button->ccMessages[i - 1].minValue, editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), 10);
            strlcpy(editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), " ", LABEL_2_LENGTH);
            itoa(button->ccMessages[i - 1].maxValue, editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), 10);
        }
    }

    for (size_t i = 0; i < 9; i++)
    {
        editLabelAttrib[i].color = 0;
    }
}

void setCc2LabelAttributes(const Button* button)
{
    strlcpy(editLabelAttrib[0].label1, "EXIT", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[1].label1, "CC #9", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[2].label1, "CC #10", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[3].label1, "CC #11", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[4].label1, "CC #12", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[5].label1, "CC #13", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[6].label1, "CC #14", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[7].label1, "CC #15", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[8].label1, "CC #16", LABEL_1_LENGTH);

    strlcpy(editLabelAttrib[0].label2, " ", LABEL_2_LENGTH);
    for (size_t i = 1; i < 9; i++)
    {
        strlcpy(editLabelAttrib[i].label2, " ", LABEL_2_LENGTH);
        if (button->ccMessages[i - 1 + 8].channel > 0)
        {
            itoa(button->ccMessages[i - 1 + 8].channel, editLabelAttrib[i].label2, 10);
            strlcpy(editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), " ", LABEL_2_LENGTH);
            itoa(button->ccMessages[i - 1 + 8].ccNumber, editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), 10);
            strlcpy(editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), " ", LABEL_2_LENGTH);
            itoa(button->ccMessages[i - 1 + 8].minValue, editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), 10);
            strlcpy(editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), " ", LABEL_2_LENGTH);
            itoa(button->ccMessages[i - 1 + 8].maxValue, editLabelAttrib[i].label2 + strlen(editLabelAttrib[i].label2), 10);
        }
    }

    for (size_t i = 0; i < 9; i++)
    {
        editLabelAttrib[i].color = 0;
    }
}

void setPcDetailLabelAttributes(const PcMsg* msg)
{
    strlcpy(editLabelAttrib[0].label1, "EXIT", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[1].label1, "CHANNEL", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[2].label1, " ", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[3].label1, " ", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[4].label1, "NBR ON", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[5].label1, "NBR OFF", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[6].label1, " ", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[7].label1, " ", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[8].label1, " ", LABEL_1_LENGTH);

    for (size_t i = 0; i < 9; i++)
        strlcpy(editLabelAttrib[i].label2, " ", LABEL_2_LENGTH);

    if (msg->channel > 0)
    {
        itoa(msg->channel, editLabelAttrib[1].label2, 10);
        itoa(msg->valueOn, editLabelAttrib[4].label2, 10);
        itoa(msg->valueOff, editLabelAttrib[5].label2, 10);
    }

    for (size_t i = 0; i < 9; i++)
    {
        editLabelAttrib[i].color = 0;
    }
}

void setCcDetailLabelAttributes(const CcMsg* msg)
{
    strlcpy(editLabelAttrib[0].label1, "EXIT", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[1].label1, "CHANNEL", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[2].label1, "CC NBR", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[3].label1, " ", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[4].label1, "MIN VAL", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[5].label1, "MAX VAL", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[6].label1, "EXP CTRL", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[7].label1, " ", LABEL_1_LENGTH);
    strlcpy(editLabelAttrib[8].label1, "MAX V CC", LABEL_1_LENGTH);

    for (size_t i = 0; i < 9; i++)
        strlcpy(editLabelAttrib[i].label2, " ", LABEL_2_LENGTH);

    if (msg->channel > 0)
    {
        itoa(msg->channel, editLabelAttrib[1].label2, 10);
        itoa(msg->ccNumber, editLabelAttrib[2].label2, 10);
        itoa(msg->minValue, editLabelAttrib[4].label2, 10);
        itoa(msg->maxValue, editLabelAttrib[5].label2, 10);
    }

    for (size_t i = 0; i < 9; i++)
    {
        editLabelAttrib[i].color = 0;
    }

    editLabelAttrib[6].color = TFT_RED;
    if (msg->ctrlByExp)
        editLabelAttrib[6].color = TFT_GREEN;

    editLabelAttrib[8].color = TFT_RED;
    if (msg->useMaxValAsCcNbr)
        editLabelAttrib[8].color = TFT_GREEN;
}