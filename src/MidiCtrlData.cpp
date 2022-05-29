#include "MidiCtrlData.h"
#include <EEPROM.h>
#include <SD.h>
#include <SPI.h>

MidiCtrlData::MidiCtrlData()
{
    folder = strdup(PREFIX_VER);
    strcat(folder, MIDI_CTRL_DATA_VERSION);

    if (verifyVersionFolderExists())
    {
        Serial.print("Version folder OK: ");
        Serial.println(folder); 
        deserialize();
    }
    else
    {
        Serial.print("Version folder ERR: ");
        Serial.println(folder);
        initialize();
        serialize();
    }  
}

Bank* MidiCtrlData::getBank(uint8_t bankNbr)
{
    return &banks[bankNbr];
}

void MidiCtrlData::saveBank(const uint8_t bankNbr)
{
    serializeBank(bankNbr);
}

void MidiCtrlData::resetBank(const uint8_t bankNbr)
{
    initializeBank(bankNbr);
    serializeBank(bankNbr);
}

void MidiCtrlData::copyBank(const uint8_t bankNbrFrom, const uint8_t bankNbrTo)
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

void MidiCtrlData::resetButton(const uint8_t bankNbr, const uint8_t buttonNbr)
{
    initializeButton(bankNbr, buttonNbr);
    serializeBank(bankNbr);
}

void MidiCtrlData::copyButton(const uint8_t bankNbrFrom, const uint8_t buttonNbrFrom, const uint8_t bankNbrTo, const uint8_t buttonNbrTo)
{
    copyButtonData(bankNbrFrom, buttonNbrFrom, bankNbrTo, buttonNbrTo);
    serializeBank(bankNbrTo);
}

void MidiCtrlData::copyButtonData(const uint8_t bankNbrFrom, const uint8_t buttonNbrFrom, const uint8_t bankNbrTo, const uint8_t buttonNbrTo)
{
    strcpy(banks[bankNbrTo].buttons[buttonNbrTo].name, banks[bankNbrFrom].buttons[buttonNbrFrom].name);
    banks[bankNbrTo].buttons[buttonNbrTo].isPatch                = banks[bankNbrFrom].buttons[buttonNbrFrom].isPatch;
    banks[bankNbrTo].buttons[buttonNbrTo].isSecondPushEnabled    = banks[bankNbrFrom].buttons[buttonNbrFrom].isSecondPushEnabled;
    banks[bankNbrTo].buttons[buttonNbrTo].isInitialToggleStateOn = banks[bankNbrFrom].buttons[buttonNbrFrom].isInitialToggleStateOn;
    banks[bankNbrTo].buttons[buttonNbrTo].isLatching             = banks[bankNbrFrom].buttons[buttonNbrFrom].isLatching;
    for (int k = 0; k < NUMBER_OF_MIDI_MSG; k++)
    {
        banks[bankNbrTo].buttons[buttonNbrTo].pcMessages[k].channel          = banks[bankNbrFrom].buttons[buttonNbrFrom].pcMessages[k].channel;
        banks[bankNbrTo].buttons[buttonNbrTo].pcMessages[k].valueOn          = banks[bankNbrFrom].buttons[buttonNbrFrom].pcMessages[k].valueOn;
        banks[bankNbrTo].buttons[buttonNbrTo].pcMessages[k].valueOff         = banks[bankNbrFrom].buttons[buttonNbrFrom].pcMessages[k].valueOff;
        banks[bankNbrTo].buttons[buttonNbrTo].ccMessages[k].channel          = banks[bankNbrFrom].buttons[buttonNbrFrom].ccMessages[k].channel;
        banks[bankNbrTo].buttons[buttonNbrTo].ccMessages[k].ccNumber         = banks[bankNbrFrom].buttons[buttonNbrFrom].ccMessages[k].ccNumber;
        banks[bankNbrTo].buttons[buttonNbrTo].ccMessages[k].minValue         = banks[bankNbrFrom].buttons[buttonNbrFrom].ccMessages[k].minValue;
        banks[bankNbrTo].buttons[buttonNbrTo].ccMessages[k].maxValue         = banks[bankNbrFrom].buttons[buttonNbrFrom].ccMessages[k].maxValue;
        banks[bankNbrTo].buttons[buttonNbrTo].ccMessages[k].useMaxValAsCcNbr = banks[bankNbrFrom].buttons[buttonNbrFrom].ccMessages[k].useMaxValAsCcNbr;
        banks[bankNbrTo].buttons[buttonNbrTo].ccMessages[k].ctrlByExp        = banks[bankNbrFrom].buttons[buttonNbrFrom].ccMessages[k].ctrlByExp;
    }
}

void MidiCtrlData::initialize()
{
    for (int i = 0; i < NUMBER_OF_BANKS; i++)
    {
        initializeBank(i);
    }
}

void MidiCtrlData::initializeBank(const uint8_t bankNbr)
{

    strcpy(banks[bankNbr].name, BANK);
    banks[bankNbr].pcMessage.channel  = 0;
    banks[bankNbr].pcMessage.valueOn  = 0;
    banks[bankNbr].pcMessage.valueOff = 0;
    for (int j = 0; j < NUMBER_OF_BUTTONS; j++)
    {
        initializeButton(bankNbr, j);
    }
}

void MidiCtrlData::initializeButton(const uint8_t bankNbr, const uint8_t buttonNbr)
{
    strcpy(banks[bankNbr].buttons[buttonNbr].name, PATCH);
    banks[bankNbr].buttons[buttonNbr].isPatch                = true;
    banks[bankNbr].buttons[buttonNbr].isSecondPushEnabled    = true;
    banks[bankNbr].buttons[buttonNbr].isInitialToggleStateOn = false;
    banks[bankNbr].buttons[buttonNbr].isLatching             = true;
    for (int k = 0; k < NUMBER_OF_MIDI_MSG; k++)
    {
        banks[bankNbr].buttons[buttonNbr].pcMessages[k].channel          = 0;
        banks[bankNbr].buttons[buttonNbr].pcMessages[k].valueOn          = 0;
        banks[bankNbr].buttons[buttonNbr].pcMessages[k].valueOff         = 0;
        banks[bankNbr].buttons[buttonNbr].ccMessages[k].channel          = 0;
        banks[bankNbr].buttons[buttonNbr].ccMessages[k].ccNumber         = 0;
        banks[bankNbr].buttons[buttonNbr].ccMessages[k].minValue         = 0;
        banks[bankNbr].buttons[buttonNbr].ccMessages[k].maxValue         = 0;
        banks[bankNbr].buttons[buttonNbr].ccMessages[k].useMaxValAsCcNbr = false;
        banks[bankNbr].buttons[buttonNbr].ccMessages[k].ctrlByExp        = false;
    }
}

void MidiCtrlData::deserialize()
{
    for (int i = 0; i < NUMBER_OF_BANKS; i++)
    {
        deserializeBank(i);
    }
}

void MidiCtrlData::deserializeBank(const uint8_t id)
{
    DynamicJsonDocument doc(ARDUINO_JSON_DOC_SIZE);
    loadBankFromFile(buildPath(id), doc);

    strlcpy(banks[id].name, doc[NAME], sizeof(banks[id].name));
    banks[id].pcMessage.channel  = doc[PC_CHANNEL];
    banks[id].pcMessage.valueOn  = doc[PC_VALUE_ON];
    banks[id].pcMessage.valueOff = doc[PC_VALUE_OFF];

    for (int j = 0; j < NUMBER_OF_BUTTONS; j++)
    {
        JsonObject btn = doc[BUTTON+String(j)];
        strlcpy(banks[id].buttons[j].name, btn[NAME], sizeof(banks[id].buttons[j].name));
        banks[id].buttons[j].isPatch                = btn[IS_PATCH];
        banks[id].buttons[j].isSecondPushEnabled    = btn[IS_SECOND_PUSH_ENABLED];
        banks[id].buttons[j].isInitialToggleStateOn = btn[IS_INITIAL_TOGGLE_STATE_ON];
        banks[id].buttons[j].isLatching             = btn[IS_LATCHING];

        for (int k = 0; k < NUMBER_OF_MIDI_MSG; k++)
        {
            JsonObject msg                                      = btn[MESSAGE + String(k)];
            banks[id].buttons[j].pcMessages[k].channel          = msg[PC_CHANNEL];
            banks[id].buttons[j].pcMessages[k].valueOn          = msg[PC_VALUE_ON];
            banks[id].buttons[j].pcMessages[k].valueOff         = msg[PC_VALUE_OFF];
            banks[id].buttons[j].ccMessages[k].channel          = msg[CC_CHANNEL];
            banks[id].buttons[j].ccMessages[k].ccNumber         = msg[CC_NUMBER];
            banks[id].buttons[j].ccMessages[k].minValue         = msg[CC_VALUE_MIN];
            banks[id].buttons[j].ccMessages[k].maxValue         = msg[CC_VALUE_MAX];
            banks[id].buttons[j].ccMessages[k].useMaxValAsCcNbr = msg[CC_VALUE_MAX_AS_CC_NBR];
            banks[id].buttons[j].ccMessages[k].ctrlByExp        = msg[CC_CTRL_BY_EXP];
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

void MidiCtrlData::serializeBank(const uint8_t id)
{
    DynamicJsonDocument doc(ARDUINO_JSON_DOC_SIZE);

    doc[NAME]         = banks[id].name;
    doc[PC_CHANNEL]   = banks[id].pcMessage.channel;
    doc[PC_VALUE_ON]  = banks[id].pcMessage.valueOn;
    doc[PC_VALUE_OFF] = banks[id].pcMessage.valueOff;

    for (int j = 0; j < NUMBER_OF_BUTTONS; j++)
    {
        JsonObject btn                  = doc.createNestedObject(BUTTON + String(j));
        btn[NAME]                       = banks[id].buttons[j].name;
        btn[IS_PATCH]                   = banks[id].buttons[j].isPatch;
        btn[IS_SECOND_PUSH_ENABLED]     = banks[id].buttons[j].isSecondPushEnabled;
        btn[IS_INITIAL_TOGGLE_STATE_ON] = banks[id].buttons[j].isInitialToggleStateOn;
        btn[IS_LATCHING]                = banks[id].buttons[j].isLatching;

        for (int k = 0; k < NUMBER_OF_MIDI_MSG; k++)
        {
            JsonObject msg              = btn.createNestedObject(MESSAGE + String(k));
            msg[PC_CHANNEL]             = banks[id].buttons[j].pcMessages[k].channel;
            msg[PC_VALUE_ON]            = banks[id].buttons[j].pcMessages[k].valueOn;
            msg[PC_VALUE_OFF]           = banks[id].buttons[j].pcMessages[k].valueOff;
            msg[CC_CHANNEL]             = banks[id].buttons[j].ccMessages[k].channel;
            msg[CC_NUMBER]              = banks[id].buttons[j].ccMessages[k].ccNumber;
            msg[CC_VALUE_MIN]           = banks[id].buttons[j].ccMessages[k].minValue;
            msg[CC_VALUE_MAX]           = banks[id].buttons[j].ccMessages[k].maxValue;
            msg[CC_VALUE_MAX_AS_CC_NBR] = banks[id].buttons[j].ccMessages[k].useMaxValAsCcNbr;
            msg[CC_CTRL_BY_EXP]         = banks[id].buttons[j].ccMessages[k].ctrlByExp;
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

char* MidiCtrlData::buildPath(const uint8_t id)
{
    char filename[10];
    strcpy(filename, PREFIX_BANK);
    itoa(id, filename + strlen(filename), 10);
    strcat(filename, ".json");

    static char path[20];
    strcpy(path, folder);
    strcat(path, "/");
    strcat(path, filename);

    return path;
}
