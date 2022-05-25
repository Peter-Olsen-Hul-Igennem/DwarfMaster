#ifndef DWARFMASTER_MIDI_CTRL_DATA_H
#define DWARFMASTER_MIDI_CTRL_DATA_H

#include "GlobalConst.h"
#include <Arduino.h>
#include <ArduinoJson.h>
/*
#define NUMBER_OF_BANKS 100
#define NUMBER_OF_BUTTONS 6
#define NUMBER_OF_MIDI_MSG 16
#define BANK_NAME_LENGTH 9
#define BUTTON_NAME_LENGTH 10
*/
struct PcMsg
{
    char channel;
    char valueOn;
    char valueOff;
};

struct CcMsg
{
    char channel;
    char ccNumber;
    char minValue;
    char maxValue;
    char useMaxValAsCcNbr;
    char ctrlByExp;
};

struct Button
{
    char name[BUTTON_NAME_LENGTH + 1];
    bool isPatch;                // (Patch or Toggle)
    bool isSecondPushEnabled;    // (always true if type is Toggle)
    bool isInitialToggleStateOn; // (always false if type is Patch)
    PcMsg pcMessages[NUMBER_OF_MIDI_MSG];
    CcMsg ccMessages[NUMBER_OF_MIDI_MSG];
};

struct Bank
{
    char name[BANK_NAME_LENGTH + 1];
    PcMsg pcMessage;
    Button buttons[NUMBER_OF_BUTTONS];
};

class MidiCtrlData
{
 public:
    MidiCtrlData();
    Bank* getBank(const uint8_t bankNbr);
    void saveBank(const uint8_t bankNbr);
    void resetBank(const uint8_t bankNbr);
    void copyBank(const uint8_t bankNbrFrom, const uint8_t bankNbrTo);
    void resetButton(const uint8_t bankNbr, const uint8_t buttonNbr);                                                      // This buttonNbr is 0 based
    void copyButton(const uint8_t bankNbrFrom, const uint8_t buttonNbrFrom, const uint8_t bankNbrTo, const uint8_t buttonNbrTo); // This buttonNbrs are 0 based

 private:
    void initialize();
    void initializeBank(const uint8_t bankNbr);
    void initializeButton(const uint8_t bankNbr, const uint8_t buttonNbr);
    void copyButtonData(const uint8_t bankNbrFrom, const uint8_t buttonNbrFrom, const uint8_t bankNbrTo, const uint8_t buttonNbrTo);
    void deserialize();
    void deserializeBank(const uint8_t id);
    void serialize();
    void serializeBank(const uint8_t id);
    void loadBankFromFile(const char* filename, DynamicJsonDocument& doc);
    void saveBankToFile(const char* filename, const DynamicJsonDocument& doc);
    char* buildPath(const uint8_t id);
    bool verifyVersionFolderExists();

    Bank banks[NUMBER_OF_BANKS];

    const char* MIDI_CTRL_DATA_VERSION   = "0.3";
    const char* PREFIX_VER               = "VER_";
    const char* PREFIX_BANK              = "BANK_";
    const uint16_t ARDUINO_JSON_DOC_SIZE = 20000;

    
    const char BANK[5] = "BANK";
    const char PATCH[6] = "PATCH";
    const char BUTTON[7] = "button";
    const char NAME[5] = "name";
    const char IS_PATCH[8] = "isPatch";
    const char IS_SECOND_PUSH_ENABLED[20] = "isSecondPushEnabled";
    const char IS_INITIAL_TOGGLE_STATE_ON[23] ="isInitialToggleStateOn";
    const char MESSAGE[8] = "message";
    const char PC_CHANNEL[10] = "pcChannel";
    const char PC_VALUE_ON[10] = "pcValueOn";
    const char PC_VALUE_OFF[11] = "pcValueOff";
    const char CC_CHANNEL[10] = "ccChannel";
    const char CC_NUMBER[9] = "ccNumber";
    const char CC_VALUE_MIN[11] = "ccMinValue";
    const char CC_VALUE_MAX[11] = "ccMaxValue";
    const char CC_VALUE_MAX_AS_CC_NBR[18] = "ccMaxValueAsCcNbr";
    const char CC_CTRL_BY_EXP[10] = "ctrlByExp";
    
    char* folder;
};

#endif
