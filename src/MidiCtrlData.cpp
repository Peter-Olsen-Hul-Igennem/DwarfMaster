#include "MidiCtrlData.h"
#include <EEPROM.h>
#include <SD.h>
#include <SPI.h>

MidiCtrlData::MidiCtrlData(const char* dataVersion)
{
    this->dataVersion = dataVersion;

    folder = strdup(prefixVER);
    strcat(folder, dataVersion);

    if (verifyVersionFolderExists())
    {
        Serial.println("Version folder OK");
        deserialize();
    }
    else
    {
        Serial.println("Version folder ERR");
        initialize();
    }
}

Bank* MidiCtrlData::getBank(byte bankNbr)
{
    return &banks[bankNbr];
}

void MidiCtrlData::saveBank(const byte bankNbr)
{
    serializeBank(bankNbr);
}

void MidiCtrlData::resetBank(const byte bankNbr)
{
    initializeBank(bankNbr);
    serializeBank(bankNbr);
}

void MidiCtrlData::copyBank(const byte bankNbrFrom, const byte bankNbrTo)
{
    strcpy(banks[bankNbrTo].name, banks[bankNbrFrom].name);
    banks[bankNbrTo].pcMessage.channel  = banks[bankNbrFrom].pcMessage.channel;
    banks[bankNbrTo].pcMessage.valueOn  = banks[bankNbrFrom].pcMessage.valueOn;
    banks[bankNbrTo].pcMessage.valueOff = banks[bankNbrFrom].pcMessage.valueOff;
    for (int j = 0; j < NUMBER_OF_BUTTONS; j++)
    {
        copyButtonData(bankNbrFrom, j, bankNbrTo, j);
    }
    
    serializeBank(bankNbrTo);
}


void MidiCtrlData::resetButton(const byte bankNbr, const byte buttonNbr)
{
    initializeButton(bankNbr, buttonNbr);
    serializeBank(bankNbr);
}

void MidiCtrlData::copyButton(const byte bankNbrFrom, const byte buttonNbrFrom, const byte bankNbrTo, const byte buttonNbrTo)
{
    copyButtonData(bankNbrFrom, buttonNbrFrom, bankNbrTo, buttonNbrTo);
    serializeBank(bankNbrTo);
}

void MidiCtrlData::copyButtonData(const byte bankNbrFrom, const byte buttonNbrFrom, const byte bankNbrTo, const byte buttonNbrTo)
{
    strcpy(banks[bankNbrTo].buttons[buttonNbrTo].name, banks[bankNbrFrom].buttons[buttonNbrFrom].name);
    banks[bankNbrTo].buttons[buttonNbrTo].isPatch                = banks[bankNbrFrom].buttons[buttonNbrFrom].isPatch;
    banks[bankNbrTo].buttons[buttonNbrTo].isSecondPushEnabled    = banks[bankNbrFrom].buttons[buttonNbrFrom].isSecondPushEnabled;
    banks[bankNbrTo].buttons[buttonNbrTo].isInitialToggleStateOn = banks[bankNbrFrom].buttons[buttonNbrFrom].isInitialToggleStateOn;
    for (int k = 0; k < NUMBER_OF_MIDI_MSG; k++)
    {
        banks[bankNbrTo].buttons[buttonNbrTo].pcMessages[k].channel  = banks[bankNbrFrom].buttons[buttonNbrFrom].pcMessages[k].channel;
        banks[bankNbrTo].buttons[buttonNbrTo].pcMessages[k].valueOn  = banks[bankNbrFrom].buttons[buttonNbrFrom].pcMessages[k].valueOn;
        banks[bankNbrTo].buttons[buttonNbrTo].pcMessages[k].valueOff = banks[bankNbrFrom].buttons[buttonNbrFrom].pcMessages[k].valueOff;
        banks[bankNbrTo].buttons[buttonNbrTo].ccMessages[k].channel  = banks[bankNbrFrom].buttons[buttonNbrFrom].ccMessages[k].channel;
        banks[bankNbrTo].buttons[buttonNbrTo].ccMessages[k].ccNumber = banks[bankNbrFrom].buttons[buttonNbrFrom].ccMessages[k].ccNumber;
        banks[bankNbrTo].buttons[buttonNbrTo].ccMessages[k].minValue = banks[bankNbrFrom].buttons[buttonNbrFrom].ccMessages[k].minValue;
        banks[bankNbrTo].buttons[buttonNbrTo].ccMessages[k].maxValue = banks[bankNbrFrom].buttons[buttonNbrFrom].ccMessages[k].maxValue;
    }
}

void MidiCtrlData::initialize()
{
    for (int i = 0; i < NUMBER_OF_BANKS; i++)
    {
        initializeBank(i);
    }
    serialize();
}    

void MidiCtrlData::initializeBank(const byte bankNbr)
{

    strcpy(banks[bankNbr].name, "BANK");
    banks[bankNbr].pcMessage.channel  = 0;
    banks[bankNbr].pcMessage.valueOn  = 0;
    banks[bankNbr].pcMessage.valueOff = 0;
    for (int j = 0; j < NUMBER_OF_BUTTONS; j++)
    {
        initializeButton(bankNbr, j);
    }
}

void MidiCtrlData::initializeButton(const byte bankNbr, const byte buttonNbr)
{
    strcpy(banks[bankNbr].buttons[buttonNbr].name, "PATCH");
    banks[bankNbr].buttons[buttonNbr].isPatch                = true;
    banks[bankNbr].buttons[buttonNbr].isSecondPushEnabled    = true;
    banks[bankNbr].buttons[buttonNbr].isInitialToggleStateOn = false;
    for (int k = 0; k < NUMBER_OF_MIDI_MSG; k++)
    {
        banks[bankNbr].buttons[buttonNbr].pcMessages[k].channel  = 0;
        banks[bankNbr].buttons[buttonNbr].pcMessages[k].valueOn  = 0;
        banks[bankNbr].buttons[buttonNbr].pcMessages[k].valueOff = 0;
        banks[bankNbr].buttons[buttonNbr].ccMessages[k].channel  = 0;
        banks[bankNbr].buttons[buttonNbr].ccMessages[k].ccNumber = 0;
        banks[bankNbr].buttons[buttonNbr].ccMessages[k].minValue = 0;
        banks[bankNbr].buttons[buttonNbr].ccMessages[k].maxValue = 0;
    }
}

void MidiCtrlData::deserialize()
{
    for (int i = 0; i < NUMBER_OF_BANKS; i++)
    {
        deserializeBank(i);
    }
}

void MidiCtrlData::deserializeBank(const int id)
{
    DynamicJsonDocument doc(arduinoJsonDocSize);
    loadBankFromFile(buildPath(id), doc);

    strlcpy(banks[id].name, doc["name"], sizeof(banks[id].name));
    banks[id].pcMessage.channel  = doc["pcChannel"];
    banks[id].pcMessage.valueOn  = doc["pcValueOn"];
    banks[id].pcMessage.valueOff = doc["pcValueOff"];

    for (int j = 0; j < NUMBER_OF_BUTTONS; j++)
    {
        JsonObject btn = doc["button" + String(j)];
        strlcpy(banks[id].buttons[j].name, btn["name"], sizeof(banks[id].buttons[j].name));
        banks[id].buttons[j].isPatch                = btn["isPatch"];
        banks[id].buttons[j].isSecondPushEnabled    = btn["isSecondPushEnabled"];
        banks[id].buttons[j].isInitialToggleStateOn = btn["isInitialToggleStateOn"];

        for (int k = 0; k < NUMBER_OF_MIDI_MSG; k++)
        {
            JsonObject msg                              = btn["message" + String(k)];
            banks[id].buttons[j].pcMessages[k].channel  = msg["pcChannel"];
            banks[id].buttons[j].pcMessages[k].valueOn  = msg["pcValueOn"];
            banks[id].buttons[j].pcMessages[k].valueOff = msg["pcValueOff"];
            banks[id].buttons[j].ccMessages[k].channel  = msg["ccChannel"];
            banks[id].buttons[j].ccMessages[k].ccNumber = msg["ccNumber"];
            banks[id].buttons[j].ccMessages[k].minValue = msg["ccMinValue"];
            banks[id].buttons[j].ccMessages[k].maxValue = msg["ccMaxValue"];
        }
    }
}

void MidiCtrlData::serialize()
{
    for (int i = 0; i < NUMBER_OF_BANKS; i++)
    {
        serializeBank(i);
    }
}

void MidiCtrlData::serializeBank(const int id)
{
    DynamicJsonDocument doc(arduinoJsonDocSize);

    doc["name"]       = banks[id].name;
    doc["pcChannel"]  = banks[id].pcMessage.channel;
    doc["pcValueOn"]  = banks[id].pcMessage.valueOn;
    doc["pcValueOff"] = banks[id].pcMessage.valueOff;

    for (int j = 0; j < NUMBER_OF_BUTTONS; j++)
    {
        JsonObject btn                = doc.createNestedObject("button" + String(j));
        btn["name"]                   = banks[id].buttons[j].name;
        btn["isPatch"]                = banks[id].buttons[j].isPatch;
        btn["isSecondPushEnabled"]    = banks[id].buttons[j].isSecondPushEnabled;
        btn["isInitialToggleStateOn"] = banks[id].buttons[j].isInitialToggleStateOn;

        for (int k = 0; k < NUMBER_OF_MIDI_MSG; k++)
        {
            JsonObject msg    = btn.createNestedObject("message" + String(k));
            msg["pcChannel"]  = banks[id].buttons[j].pcMessages[k].channel;
            msg["pcValueOn"]  = banks[id].buttons[j].pcMessages[k].valueOn;
            msg["pcValueOff"] = banks[id].buttons[j].pcMessages[k].valueOff;
            msg["ccChannel"]  = banks[id].buttons[j].ccMessages[k].channel;
            msg["ccNumber"]   = banks[id].buttons[j].ccMessages[k].ccNumber;
            msg["ccMinValue"] = banks[id].buttons[j].ccMessages[k].minValue;
            msg["ccMaxValue"] = banks[id].buttons[j].ccMessages[k].maxValue;
        }
    }
    // serializeJsonPretty(doc, Serial);
    // Serial.println();

    saveBankToFile(buildPath(id), doc);
}

void MidiCtrlData::loadBankFromFile(const char* path, DynamicJsonDocument& doc)
{

    if (!SD.begin(BUILTIN_SDCARD))
    {
        Serial.println("SD initialization failed!");
    }

    File file = SD.open(path);
    if (!file)
    {
        Serial.println(F("Failed to create file"));
        Serial.println(path);
    }

    DeserializationError error = deserializeJson(doc, file);
    if (error)
        Serial.println(F("Failed to read file, using default configuration"));

    file.close();
}

void MidiCtrlData::saveBankToFile(const char* path, const DynamicJsonDocument& doc)
{

    if (!SD.begin(BUILTIN_SDCARD))
    {
        Serial.println("SD initialization failed!");
        return;
    }

    if (!SD.exists(folder))
        SD.mkdir(folder);

    // Delete existing file, otherwise the configuration is appended to the file
    SD.remove(path);

    // Open file for writing
    File file = SD.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println(F("Failed to create file"));
        Serial.println(path);
        return;
    }

    // Serialize JSON to file
    if (serializeJson(doc, file) == 0)
    {
        Serial.println(F("Failed to write to file"));
    }

    // Close the file
    file.close();
}

bool MidiCtrlData::verifyVersionFolderExists()
{
    if (!SD.begin(BUILTIN_SDCARD))
    {
        Serial.println("SD initialization failed!");
        return false;
    }

    return SD.exists(folder);
}

char* MidiCtrlData::buildPath(int id)
{
    char filename[10];
    strcpy(filename, prefixBANK);
    itoa(id, filename + strlen(filename), 10);
    strcat(filename, ".txt");

    static char path[20];
    strcpy(path, folder);
    strcat(path, "/");
    strcat(path, filename);

    return path;
}
