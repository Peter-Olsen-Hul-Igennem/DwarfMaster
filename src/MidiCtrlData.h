#ifndef DWARFMASTER_MIDI_CTRL_DATA_H
#define DWARFMASTER_MIDI_CTRL_DATA_H

#include <Arduino.h>
#include <ArduinoJson.h>

#define NUMBER_OF_BANKS 100
#define NUMBER_OF_BUTTONS 6
#define NUMBER_OF_MIDI_MSG 16
#define BANK_NAME_LENGTH 9
#define BUTTON_NAME_LENGTH 10

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
    MidiCtrlData(const char* dataVersion);
    Bank* getBank(const byte bankNbr);
    void saveBank(const byte bankNbr);

 private:
    void initialize();
    void deserialize();
    void deserializeBank(const int id);
    void serialize();
    void serializeBank(const int id);
    void loadBankFromFile(const char* filename, DynamicJsonDocument& doc);
    void saveBankToFile(const char* filename, const DynamicJsonDocument& doc);
    char* buildPath(int id);
    bool verifyVersionFolderExists();

    Bank banks[NUMBER_OF_BANKS];

    const char* dataVersion;
    const char* prefixVER        = "VER_";
    const char* prefixBANK       = "BANK_";
    const int arduinoJsonDocSize = 15000;

    char* folder;
};

#endif
