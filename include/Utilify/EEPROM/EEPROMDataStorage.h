#pragma once

#include <Arduino.h>
#include <CRC16.h>
#include <EEPROM.h>

#include <Utilify/TerminalConstants.h>
#include <Utilify/StringUtils.h>

#include "DataStorage.h"

// EEPROM organization:
// || app signature (2 bytes) || start address (2 bytes) || 
// || write counter (2 bytes) || user data CRC (2 bytes) || 
// || user data (sizeof(DataType)) || ...

template <typename DataType>
class EEPROMDataStorage : public DataStorage<DataType> {
public:
    explicit EEPROMDataStorage(const uint16_t& appSignature);

    virtual bool save() noexcept override;
    virtual bool load() noexcept override;

    void initIfFirstTime();
    void eraseEeprom();

    static void dumpEepromContent();
    static uint16_t dataBlockSize();

private:
    static uint16_t addressWriteCounter(const uint16_t& dataAddress);
    static uint16_t addressDataCRC(const uint16_t& dataAddress);
    static uint16_t addressUserData(const uint16_t& dataAddress);

    const uint16_t APP_SIGNATURE;

    static const uint16_t ADDRESS_APP_SIGNATURE;
    static const uint16_t ADDRESS_START_DATA;
    static const uint16_t INITIAL_START_DATA_ADDRESS;
    static const uint16_t MAX_WRITES_PER_BLOCK;
};

// Static constants
template <typename DataType>
const uint16_t EEPROMDataStorage<DataType>::ADDRESS_APP_SIGNATURE = 0;
template <typename DataType>
const uint16_t EEPROMDataStorage<DataType>::ADDRESS_START_DATA = 2;
template <typename DataType>
const uint16_t EEPROMDataStorage<DataType>::INITIAL_START_DATA_ADDRESS = 4;

#ifdef OVERRIDE_EEPROM_MAX_WRITES_PER_BLOCK
template <typename DataType>
const uint16_t EEPROMDataStorage<DataType>::MAX_WRITES_PER_BLOCK = OVERRIDE_EEPROM_MAX_WRITES_PER_BLOCK;
#else
template <typename DataType>
const uint16_t EEPROMDataStorage<DataType>::MAX_WRITES_PER_BLOCK = 20000;
#endif

// Constructor
template <typename DataType>
EEPROMDataStorage<DataType>::EEPROMDataStorage(const uint16_t& appSignature)
    : APP_SIGNATURE(appSignature) {}

// Size calculation
template <typename DataType>
uint16_t EEPROMDataStorage<DataType>::dataBlockSize() {
    return sizeof(uint16_t) + sizeof(uint16_t) + sizeof(DataType);
}

// Address calculations
template <typename DataType>
uint16_t EEPROMDataStorage<DataType>::addressWriteCounter(const uint16_t& dataAddress) {
    return dataAddress;
}

template <typename DataType>
uint16_t EEPROMDataStorage<DataType>::addressDataCRC(const uint16_t& dataAddress) {
    return addressWriteCounter(dataAddress) + sizeof(uint16_t);
}

template <typename DataType>
uint16_t EEPROMDataStorage<DataType>::addressUserData(const uint16_t& dataAddress) {
    return addressDataCRC(dataAddress) + sizeof(uint16_t);
}

// Save to EEPROM
template <typename DataType>
bool EEPROMDataStorage<DataType>::save() noexcept {
    bool success = true;
    uint16_t dataAddress = 0;
    uint16_t writeCounter = 0;

    initIfFirstTime();
    EEPROM.get(ADDRESS_START_DATA, dataAddress);
    EEPROM.get(addressWriteCounter(dataAddress), writeCounter);

    if (writeCounter >= MAX_WRITES_PER_BLOCK) {
        dataAddress += dataBlockSize();
        if (dataAddress + dataBlockSize() > EEPROM.length()) {
            success = false;
        }
        writeCounter = 0;
        if (success) {
            EEPROM.put(ADDRESS_START_DATA, dataAddress);
        }
    }

    ++writeCounter;
    if (success) {
        CRC16 crc;
        crc.add((uint8_t*)&this->data(), sizeof(DataType));

        EEPROM.put(addressWriteCounter(dataAddress), writeCounter);
        EEPROM.put(addressDataCRC(dataAddress), crc.calc());
        EEPROM.put(addressUserData(dataAddress), this->data());

        success = this->load();
        CRC16 crcCheck;
        crcCheck.add((uint8_t*)&(this->data()), sizeof(DataType));
        success = crcCheck.calc() == crc.calc();
    }

    return success;
}

// Load from EEPROM
template <typename DataType>
bool EEPROMDataStorage<DataType>::load() noexcept {
    bool success = true;
    uint16_t dataAddress = 0;
    uint16_t savedCRC = 0;
    DataType dataBuffer;

    initIfFirstTime();
    EEPROM.get(ADDRESS_START_DATA, dataAddress);
    EEPROM.get(addressDataCRC(dataAddress), savedCRC);
    EEPROM.get(addressUserData(dataAddress), dataBuffer);

    CRC16 crc;
    crc.add((uint8_t*)&dataBuffer, sizeof(DataType));
    success = savedCRC == crc.calc();

    if (success) {
        this->setData(dataBuffer);
    }

    return success;
}

// Initialize EEPROM if first use
template <typename DataType>
void EEPROMDataStorage<DataType>::initIfFirstTime() {
    uint16_t appSignatureInEeprom = 0;
    EEPROM.get(ADDRESS_APP_SIGNATURE, appSignatureInEeprom);

    if (appSignatureInEeprom != APP_SIGNATURE) {
        eraseEeprom();
        EEPROM.put(ADDRESS_APP_SIGNATURE, APP_SIGNATURE);
        EEPROM.put(ADDRESS_START_DATA, INITIAL_START_DATA_ADDRESS);
    }
}

// Erase EEPROM
template <typename DataType>
void EEPROMDataStorage<DataType>::eraseEeprom() {
    for (uint16_t address = 0; address < EEPROM.length(); ++address) {
        EEPROM.write(address, 0);
    }
}

// Dump EEPROM content
template <typename DataType>
void EEPROMDataStorage<DataType>::dumpEepromContent() {
    uint16_t dataAddress = 0;
    uint16_t writeCounter = 0;
    uint16_t appSignature = 0;
    uint16_t userDataCRC = 0;

    EEPROM.get(ADDRESS_APP_SIGNATURE, appSignature);
    EEPROM.get(ADDRESS_START_DATA, dataAddress);
    EEPROM.get(addressWriteCounter(dataAddress), writeCounter);
    EEPROM.get(addressDataCRC(dataAddress), userDataCRC);

    Serial.println("Data block size: " + String(dataBlockSize()));
    Serial.println("App signature: 0x" + StringUtils::padLeft(String(appSignature, HEX), 4, '0'));
    Serial.println("Start data address: 0x" + StringUtils::padLeft(String(dataAddress, HEX), 4, '0'));
    Serial.println("User data CRC address: 0x" + StringUtils::padLeft(String(addressDataCRC(dataAddress), HEX), 4, '0'));
    Serial.println("User data start address: 0x" + StringUtils::padLeft(String(addressUserData(dataAddress), HEX), 4, '0'));
    Serial.println("Write counter: " + String(writeCounter));
    Serial.println("User data CRC: 0x" + StringUtils::padLeft(String(userDataCRC, HEX), 4, '0'));
    Serial.println();

    Serial.println("EEPROM Content:");
    Serial.println("Dump capacity: " + String(EEPROM.length()));

    const uint16_t lineWidth = 16;
    Serial.print(StringUtils::padLeft("", 4));
    for (uint16_t i = 0; i < lineWidth; ++i) {
        Serial.print("  " + StringUtils::padLeft(String(i, HEX), 2, '0'));
    }
    Serial.println();

    for (uint16_t address = 0; address < EEPROM.length(); address += lineWidth) {
        Serial.print(StringUtils::padLeft(String(address, HEX), 4, '0'));
        for (uint16_t offset = 0; offset < lineWidth && (address + offset) < EEPROM.length(); ++offset) {
            uint16_t currentAddress = address + offset;

#if defined(USE_COLOR_TERMINAL)
            if (currentAddress == ADDRESS_APP_SIGNATURE ||
                currentAddress == ADDRESS_APP_SIGNATURE + 1) {
                Serial.print(TEXT_BOLD TEXT_BG_BLACK TEXT_BLUE);
            } else if (currentAddress == ADDRESS_START_DATA ||
                       currentAddress == ADDRESS_START_DATA + 1) {
                Serial.print(TEXT_BOLD TEXT_BG_BLACK TEXT_YELLOW);
            } else if (currentAddress == dataAddress ||
                       currentAddress == dataAddress + 1) {
                Serial.print(TEXT_BOLD TEXT_BG_BLACK TEXT_BLUE);
            } else if (currentAddress == addressDataCRC(dataAddress) ||
                       currentAddress == addressDataCRC(dataAddress) + 1) {
                Serial.print(TEXT_BOLD TEXT_BG_BLACK TEXT_YELLOW);
            } else if (currentAddress >= addressUserData(dataAddress) &&
                       currentAddress < addressUserData(dataAddress) + sizeof(DataType)) {
                Serial.print(TEXT_BOLD TEXT_BG_BLACK TEXT_GREEN);
            }
#endif
            Serial.print("  " + StringUtils::padLeft(String(EEPROM.read(currentAddress), HEX), 2, '0'));
#if defined(USE_COLOR_TERMINAL)
            Serial.print(TEXT_RESET);
#endif
        }

        Serial.print("  |  ");
        for (uint16_t offset = 0; offset < lineWidth && (address + offset) < EEPROM.length(); ++offset) {
            char character = (char)EEPROM.read(address + offset);
            Serial.print(isprint(character) ? character : ' ');
        }
        Serial.println("  |");
    }
    Serial.println();
}
