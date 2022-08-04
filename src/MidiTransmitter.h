#ifndef DWARFMASTER_MIDI_TRANSMITTER_H
#define DWARFMASTER_MIDI_TRANSMITTER_H

#include "MidiCtrlData.h"
#include "ExpressionHandler.h"
#include <Arduino.h>


class MidiTransmitter
{
 public:
    MidiTransmitter(ExpressionHandler* expressionHandler);
    void sendPcMsg(const PcMsg msg, const bool first);
    void sendCcMsg(const CcMsg msg, const bool first, const bool maxValAsCc);
    void sendExpressionMessages(const Bank* bank, const bool* toggleStates, const uint8_t currentSelectedBtn);

 private:
    ExpressionHandler* expHandler;
};

#endif