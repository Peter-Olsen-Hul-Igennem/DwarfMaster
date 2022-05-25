#include "MidiTransmitter.h"

MidiTransmitter::MidiTransmitter(ExpressionHandler* expressionHandler)
{
    this->expHandler = expressionHandler;
}

void MidiTransmitter::sendPcMsg(const PcMsg msg, const bool first)
{
    if (msg.channel == 0) // Midichannel == 0: Disabled. 
        return;

    if (first)
        usbMIDI.sendProgramChange(msg.valueOn, msg.channel);
    else
        usbMIDI.sendProgramChange(msg.valueOff, msg.channel);
}

void MidiTransmitter::sendCcMsg(const CcMsg msg, const bool first, const bool maxValAsCc)
{
    if (msg.channel == 0) // Midichannel == 0: Disabled. 
        return;
    
    if (msg.ctrlByExp) // CtrlByExp: Controlled continously by expression pedal.
        return;
    
    if (first)
    {
        usbMIDI.sendControlChange(msg.ccNumber, msg.minValue, msg.channel);
    }
    else
    {
        if (maxValAsCc)
            usbMIDI.sendControlChange(msg.maxValue, msg.minValue, msg.channel);
        else
            usbMIDI.sendControlChange(msg.ccNumber, msg.minValue, msg.channel);
    }
}

void MidiTransmitter::sendExpressionMessages(const Bank* bank, const bool* toggleStates, const uint8_t currentSelectedBtn)
{
    uint8_t expVal;
    if (!expHandler->readAndMapValue(&expVal))
        return;
    
    uint8_t mappedValue;
    for (uint8_t i = 0; i < NUMBER_OF_BUTTONS; i++)
    {
        if ((!bank->buttons[i].isPatch && toggleStates[i]) || i + 1 == currentSelectedBtn) // Button is a toggle that is on, or the currently selected patch
        {
            for (uint8_t j = 0; j < NUMBER_OF_MIDI_MSG; j++)
            {
                if (bank->buttons[i].ccMessages[j].ctrlByExp)
                {
                    mappedValue = map(expVal, 0, 127, bank->buttons[i].ccMessages[j].minValue, bank->buttons[i].ccMessages[j].maxValue);
                    usbMIDI.sendControlChange(bank->buttons[i].ccMessages[j].ccNumber, mappedValue, bank->buttons[i].ccMessages[j].channel);
                }
            }
        }
    }
}
/*
bool MidiTransmitter::readAndMapValue(uint8_t* result)
{
    int readValue = analogRead(expPin);
    
    if(abs(readValue - prevReadValue) < POT_THRESHOLD) // small potentiometer differences (noise) are disregarded
        return false;
    prevReadValue = readValue;
 
    uint8_t expVal = map(readValue, 0, 1013, 0, 127);
    if (expVal == prevExpValue) // if read value is mapped to the same value as the previous one it's disregarded
        return false;

    prevExpValue = expVal;

    for(uint8_t i=0; i<HIST_BUFFER_LENGTH; i++) // if the mapped value is in the "history buffer" it's disregarded, this "smoothes" the action of the pedal.
    {
        if(histBuffer[i] == expVal)
            return false;
    }
    memcpy(&histBuffer[0], &histBuffer[1], sizeof(uint8_t) * (HIST_BUFFER_LENGTH - 1)); // moving the content of the buffer one element back
    histBuffer[HIST_BUFFER_LENGTH - 1] = expVal;

    *result = expVal;

    return true;
}*/