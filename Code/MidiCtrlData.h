#ifndef MIDI_CTRL_DATA_H
#define MIDI_CTRL_DATA_H

#include <Arduino.h>
#include <ArduinoJson.h>

#define NUMBER_OF_BANKS    100
#define NUMBER_OF_BUTTONS    6
#define NUMBER_OF_MIDI_MSG  16
#define BANK_NAME_LENGTH    10
#define BUTTON_NAME_LENGTH  10

struct pcMsg
{
  char channel;
  char valueOn;
  char valueOff;
};

struct ccMsg
{
  char channel;
  char ccNumber;
  char valueOn;
  char valueOff;
};

struct button
{
  char name[BUTTON_NAME_LENGTH + 1];
  bool isPatch; // (Patch or Toggle)
  bool isSecondPushEnabled; // (always true if type is Toggle)
  bool isInitialToggleStateOn; // (always false if type is Patch)
  pcMsg pcMessages[NUMBER_OF_MIDI_MSG];
  ccMsg ccMessages[NUMBER_OF_MIDI_MSG];
};

struct bank
{
  char name[BANK_NAME_LENGTH + 1];
  pcMsg pcMessage;
  button buttons[NUMBER_OF_BUTTONS];

};

struct ctrlState
{
  int bnk;
  int btn;
};

class MidiCtrlData
{
  public:
    MidiCtrlData(const char* dataVersion);
    bank* getBank(int bankNbr);
    
  private:
    void initialize();
    void deserialize();
    void deserializeBank(const int id);
    void serialize();
    void serializeBank(const int id);
    void loadBankFromFile(const char *filename, DynamicJsonDocument &doc);
    void saveBankToFile(const char *filename, const DynamicJsonDocument &doc);
    char* buildPath(int id);
    bool verifyVersionFolderExists(); 
    void loadStateInfo(); // state is persisted to be able to start in the same state as when the controller was powered off.
    void saveStateInfo();
    
    bank banks[NUMBER_OF_BANKS];
    
    const char* dataVersion;
    const char* prefixVER = "VER_";
    const char* prefixBANK = "BANK_";
    const int arduinoJsonDocSize = 15000;
    const int stateEepromAddress = 0;

    char* folder;
    ctrlState currentState;
    
};

#endif
