#ifndef DWARFMASTER_MIDI_TRANSMITTER_H
#define DWARFMASTER_MIDI_TRANSMITTER_H

#include <Arduino.h>
#include "MidiCtrlData.h"

class MidiTransmitter
{
 public:
    void sendPcMsg(const PcMsg msg, const bool first);
    void sendCcMsg(const CcMsg msg, const bool first, const bool maxValAsCc);
    void sendExpressionMessages(const uint8_t expPin, const Bank* bank, const bool* toggleStates, const uint8_t currentSelectedBtn);

 private:
    uint8_t prevExpValue = 128;
    uint64_t prevReadTime = 0;
    
};

#endif