#ifndef DWARFMASTER_EXPRESSION_HANDLER_H
#define DWARFMASTER_EXPRESSION_HANDLER_H

#include <Arduino.h>

class ExpressionHandler
{
 public:
    ExpressionHandler(const uint8_t expressionPin, const uint16_t minCalibratedValue, const uint16_t maxCalibratedValue);
    int readAndMapValue();
    uint16_t readCalibrationValue(const bool max);

 private:
    const uint8_t POT_THRESHOLD = 4;
    static const uint8_t HIST_BUFFER_LENGTH = 4;

    uint8_t exprPin;
    uint16_t minCalibratedVal;
    uint16_t maxCalibratedVal;

    uint8_t histBuffer[HIST_BUFFER_LENGTH];
    int prevReadValue = 0;
    uint8_t prevExpValue  = 128;
};

#endif