#include "MidiTransmitter.h"
#include <MIDI.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
    
MidiTransmitter::MidiTransmitter(ExpressionHandler* expressionHandler)
{
    this->expHandler = expressionHandler;
    MIDI.begin(MIDI_CHANNEL_OFF);
}

void MidiTransmitter::sendPcMsg(const PcMsg msg, const bool first)
{
    if (msg.channel == 0) // Midichannel == 0: Disabled. 
        return;

    if (first)
    {
        usbMIDI.sendProgramChange(msg.valueOn, msg.channel);
        MIDI.sendProgramChange(msg.valueOn, msg.channel);
    }
    else
    {
        usbMIDI.sendProgramChange(msg.valueOff, msg.channel);
        MIDI.sendProgramChange(msg.valueOn, msg.channel);
    }    
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
        MIDI.sendControlChange(msg.ccNumber, msg.minValue, msg.channel);
    }
    else
    {
        if (maxValAsCc)
        {
            usbMIDI.sendControlChange(msg.maxValue, msg.minValue, msg.channel);
            MIDI.sendControlChange(msg.maxValue, msg.minValue, msg.channel);
        }
        else
        {
            usbMIDI.sendControlChange(msg.ccNumber, msg.maxValue, msg.channel);
            MIDI.sendControlChange(msg.ccNumber, msg.maxValue, msg.channel);
        }
    }
}

void MidiTransmitter::sendExpressionMessages(const Bank* bank, const bool* toggleStates, const uint8_t currentSelectedBtn)
{    
    uint8_t expVal = expHandler->readAndMapValue();
    if (-1 == expVal)
        return;

    uint8_t mappedValue;
    for (uint8_t i = 0; i < NUMBER_OF_BUTTONS; i++)
    {
        if ((!bank->buttons[i].isPatch && toggleStates[i]) || i + 1 == currentSelectedBtn) // Button is a toggle that is on, or the currently selected patch
        {
            for (uint8_t j = 0; j < NUMBER_OF_MIDI_MSG; j++)
            {
                if (bank->buttons[i].ccMessages[j].channel != 0 && bank->buttons[i].ccMessages[j].ctrlByExp) // Midichannel == 0: Disabled. 
                {
                    mappedValue = map(expVal, 0, 127, bank->buttons[i].ccMessages[j].minValue, bank->buttons[i].ccMessages[j].maxValue);
                    usbMIDI.sendControlChange(bank->buttons[i].ccMessages[j].ccNumber, mappedValue, bank->buttons[i].ccMessages[j].channel);
                    MIDI.sendControlChange(bank->buttons[i].ccMessages[j].ccNumber, mappedValue, bank->buttons[i].ccMessages[j].channel);
                }
            }
        }
    }
}
