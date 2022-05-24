#include "MidiTransmitter.h"

void MidiTransmitter::sendPcMsg(const PcMsg msg, const bool first)
{
    if (first)
        usbMIDI.sendProgramChange(msg.valueOn, msg.channel);
    else
        usbMIDI.sendProgramChange(msg.valueOff, msg.channel);
}

void MidiTransmitter::sendCcMsg(const CcMsg msg, const bool first, const bool maxValAsCc)
{
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

void MidiTransmitter::sendExpressionMessages(const uint8_t expPin, const Bank* bank, const bool* toggleStates, const uint8_t currentSelectedBtn)
{
    uint64_t now = millis();
    
    if (now - prevReadTime < 100) // Limiting the number of reads to 10 pr. second.
        return;
    
    uint8_t expVal = map(analogRead(expPin), 0, 1013, 0, 127);
    if (expVal == prevExpValue)
        return;
    
    Serial.print(expVal);
    Serial.print(" ");
    Serial.println(now - prevReadTime);
    prevReadTime = now;
    prevExpValue = expVal;

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