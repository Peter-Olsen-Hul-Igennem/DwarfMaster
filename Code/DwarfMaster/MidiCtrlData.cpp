#include <SD.h>
#include <SPI.h>
#include <EEPROM.h>
#include "MidiCtrlData.h"


MidiCtrlData::MidiCtrlData(const char* dataVersion)
{
  this->dataVersion = dataVersion;
  currentState.bnk = 0;
  currentState.btn = 0;
  
  folder = strdup(prefixVER);
  strcat(folder, dataVersion);
  
  if (verifyVersionFolderExists())
  {
    Serial.println("Version folder OK");
    deserialize();
    loadStateInfo();
  }
  else
  {
    Serial.println("Version folder ERR");
    initialize();
  }
}

bank* MidiCtrlData::getBank(int bankNbr)
{
  currentState.bnk = bankNbr;
  saveStateInfo();
  return &banks[bankNbr];
}

void MidiCtrlData::initialize()
{
  for (int i = 0; i < NUMBER_OF_BANKS; i++)
  {
    //char bankname[BANK_NAME_LENGTH + 1] = "BANK ";
    //itoa(i, bankname + strlen(bankname), 10);
    //strcpy(banks[i].name, bankname);
    strcpy(banks[i].name, "BANK");
    banks[i].pcMessage.channel = 0;
    banks[i].pcMessage.valueOn = 0;
    banks[i].pcMessage.valueOff = 0;
    for (int j = 0; j < NUMBER_OF_BUTTONS; j++)
    {
      strcpy(banks[i].buttons[j].name, "PATCH");
      banks[i].buttons[j].isPatch = true;
      banks[i].buttons[j].isSecondPushEnabled = false;
      banks[i].buttons[j].isInitialToggleStateOn = false;
      for (int k = 0; k < NUMBER_OF_MIDI_MSG; k++)
      {
        banks[i].buttons[j].pcMessages[k].channel = 0;
        banks[i].buttons[j].pcMessages[k].valueOn = 0;
        banks[i].buttons[j].pcMessages[k].valueOff = 0;
        banks[i].buttons[j].ccMessages[k].channel = 0;
        banks[i].buttons[j].ccMessages[k].ccNumber = 0;
        banks[i].buttons[j].ccMessages[k].valueOn = 0;
        banks[i].buttons[j].ccMessages[k].valueOff = 0;
      }
    }
  }
  serialize();

  currentState.bnk = 0;
  currentState.btn = 0;
  saveStateInfo();
}

void MidiCtrlData::deserialize()
{
  //return false;
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
  banks[id].pcMessage.channel = doc["pcChannel"];
  banks[id].pcMessage.valueOn = doc["pcValueOn"];
  banks[id].pcMessage.valueOff = doc["pcValueOff"];

  for (int j = 0; j < NUMBER_OF_BUTTONS; j++)
  {
    JsonObject btn = doc["button" + String(j)];
    strlcpy(banks[id].buttons[j].name, btn["name"], sizeof(banks[id].buttons[j].name));
    banks[id].buttons[j].isPatch = btn["isPatch"];
    banks[id].buttons[j].isSecondPushEnabled = btn["isSecondPushEnabled"];
    banks[id].buttons[j].isInitialToggleStateOn = btn["isInitialToggleStateOn"];
    
    for (int k = 0; k < NUMBER_OF_MIDI_MSG; k++)
    {
      JsonObject msg = btn["message" + String(k)];
      banks[id].buttons[j].pcMessages[k].channel = msg["pcChannel"];
      banks[id].buttons[j].pcMessages[k].valueOn = msg["pcValueOn"];
      banks[id].buttons[j].pcMessages[k].valueOff = msg["pcValueOff"];
      banks[id].buttons[j].ccMessages[k].channel = msg["ccChannel"];
      banks[id].buttons[j].ccMessages[k].ccNumber = msg["ccNumber"];
      banks[id].buttons[j].ccMessages[k].valueOn = msg["ccValueOn"];
      banks[id].buttons[j].ccMessages[k].valueOff = msg["ccValueOff"];
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

  doc["name"] = banks[id].name;
  doc["pcChannel"] = banks[id].pcMessage.channel;
  doc["pcValueOn"] = banks[id].pcMessage.valueOn;
  doc["pcValueOff"] = banks[id].pcMessage.valueOff;

  for (int j = 0; j < NUMBER_OF_BUTTONS; j++)
  {
    JsonObject btn = doc.createNestedObject("button" + String(j));
    btn["name"] = banks[id].buttons[j].name;
    btn["isPatch"] = banks[id].buttons[j].isPatch;
    btn["isSecondPushEnabled"] = banks[id].buttons[j].isSecondPushEnabled;
    btn["isInitialToggleStateOn"] = banks[id].buttons[j].isInitialToggleStateOn;

    for (int k = 0; k < NUMBER_OF_MIDI_MSG; k++)
    {
      JsonObject msg = btn.createNestedObject("message" + String(k));
      msg["pcChannel"] = banks[id].buttons[j].pcMessages[k].channel;
      msg["pcValueOn"] = banks[id].buttons[j].pcMessages[k].valueOn;
      msg["pcValueOff"] = banks[id].buttons[j].pcMessages[k].valueOff;
      msg["ccChannel"] = banks[id].buttons[j].ccMessages[k].channel;
      msg["ccNumber"] = banks[id].buttons[j].ccMessages[k].ccNumber;
      msg["ccValueOn"] = banks[id].buttons[j].ccMessages[k].valueOn;
      msg["ccValueOff"] = banks[id].buttons[j].ccMessages[k].valueOff;
    }
  }
  serializeJsonPretty(doc, Serial);
  Serial.println();
  //Serial.println(doc.memoryUsage());

  saveBankToFile(buildPath(id), doc);
}



void MidiCtrlData::loadBankFromFile(const char *path, DynamicJsonDocument &doc) {
  
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("SD initialization failed!");
   // return false;
  }
  //Serial.println("SD initialization ok");

  File file = SD.open(path);
  if (!file) {
    Serial.println(F("Failed to create file"));
    Serial.println(path);
   // return false;
  }
  //Serial.println("SD File created");

  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));
/*
  while (file.available()) {
    Serial.write(file.read());
  }*/
  // close the file:
  file.close();

 // return true;
}

void MidiCtrlData::saveBankToFile(const char *path, const DynamicJsonDocument &doc) {
  
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("SD initialization failed!");
    return;
  }
  
  if (!SD.exists(folder))
    SD.mkdir(folder);

  // Delete existing file, otherwise the configuration is appended to the file
  SD.remove(path);

  // Open file for writing
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.println(F("Failed to create file"));
    Serial.println(path);
    return;
  }

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  file.close();
}

bool MidiCtrlData::verifyVersionFolderExists()
{
  if (!SD.begin(BUILTIN_SDCARD)) {
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

void MidiCtrlData::loadStateInfo()
{
  EEPROM.get(stateEepromAddress, currentState);
  if (currentState.bnk < 0 || currentState.bnk > NUMBER_OF_BANKS - 1)
    currentState.bnk = 0;
  if (currentState.btn < 0 || currentState.btn > NUMBER_OF_BUTTONS - 1)
    currentState.btn = 0;
}

void MidiCtrlData::saveStateInfo()
{
  EEPROM.put(stateEepromAddress, currentState);
}
/*
char* MidiCtrlData::buildFilename(int id)
{
  static char filename[10];
  strcpy(filename, prefixBANK);
  itoa(id, filename + strlen(filename), 10);
  strcat(filename, ".txt");
  return filename;
}
*/
