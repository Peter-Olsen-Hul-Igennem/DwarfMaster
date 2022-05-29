#include "ExpressionHandler.h"

ExpressionHandler::ExpressionHandler(const uint8_t expressionPin, const uint16_t minCalibratedValue, const uint16_t maxCalibratedValue)
{
    exprPin = expressionPin;
    minCalibratedVal = minCalibratedValue;
    maxCalibratedVal = maxCalibratedValue;
}
    
bool ExpressionHandler::readAndMapValue(uint8_t* result)
{
    int readValue = analogRead(19);
    if(abs(readValue - prevReadValue) < POT_THRESHOLD) // small potentiometer differences (noise) are disregarded
        return false;
    
    prevReadValue = readValue;

    uint8_t expVal = map(readValue, minCalibratedVal, maxCalibratedVal, 0, 127);
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
}

uint16_t ExpressionHandler::readCalibrationValue(const bool max)
{
    int values = 0;
    for (uint8_t i = 0; i < 10; i++)
    {
        values += analogRead(exprPin);
    }
    
    uint16_t result = abs(values / 10);
    if (max)
        maxCalibratedVal = result;
    else
        minCalibratedVal = result;

    return result;
}