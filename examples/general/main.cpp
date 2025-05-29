#include <Arduino.h>
#include <EEPROM.h>

#define USE_COLOR_TERMINAL
#define OVERRIDE_EEPROM_MAX_WRITES_PER_BLOCK 2 // For testing purposes
#include "Utilify/EEPROM/EEPROMDataStorage.h"

struct MyData {
    int a;
    int b;
    char c[16];
};

void printData(const MyData& data) {
    Serial.println("Data content:");
    Serial.print("a = 0x");
    Serial.println(data.a, HEX);
    Serial.print("b = 0x");
    Serial.println(data.b, HEX);
    Serial.print("c = ");
    Serial.println(data.c);
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    while (!Serial); // Wait for Serial ready on boards like Leonardo

    MyData myData;
    myData.a = 0x1234;
    myData.b = 0x4321;
    strcpy(myData.c, "Chocolatine!");

    EEPROMDataStorage<MyData> eepromStorage(0x0666);

    eepromStorage.dumpEepromContent();

    constexpr size_t maxAttempts = 4;
    for (size_t i = 0; i < maxAttempts; ++i) {
        eepromStorage.setData(myData);

        if (eepromStorage.save()) {
            Serial.println("Save successful.");
        } else {
            Serial.println("Save failed.");
            break;
        }

        if (eepromStorage.load()) {
            Serial.println("Load successful.");
        } else {
            Serial.println("Load failed.");
            break;
        }

        printData(eepromStorage.data());
        eepromStorage.dumpEepromContent();
    }

    // Final verification after all writes
    if (eepromStorage.load()) {
        Serial.println("Final load successful.");
    } else {
        Serial.println("Final load failed.");
    }

    printData(eepromStorage.data());
}

void loop() {
    // Nothing to do in loop
}
