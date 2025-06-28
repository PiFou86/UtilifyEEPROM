# EEPROMDataStorage

**EEPROMDataStorage** is a lightweight and reliable EEPROM data management library designed for embedded systems, including ESP32 and Arduino Uno.  
It simplifies saving and loading structured data with automatic CRC validation to ensure data integrity.

[![PlatformIO Registry](https://badges.registry.platformio.org/packages/sirdrako/library/UtilifyEEPROM.svg)](https://registry.platformio.org/libraries/sirdrako/UtilifyEEPROM)

## Features

- **Automatic CRC Checking**: Verifies the integrity of stored data.
- **Write Counter**: Spreads writes across EEPROM to minimize memory wear.
- **Automatic Initialization**: Automatically resets EEPROM on first use if needed.
- **Lightweight Design**: Suitable for memory-constrained devices.
- **Platform Support**: Designed for **ESP32** and **Arduino Uno**.

## Getting Started

### Installation

Add **EEPROMDataStorage** to your PlatformIO project by adding the following to your `platformio.ini` file:

```ini
lib_deps =
    sirdrako/Utilify
    sirdrako/UtilifyEEPROM
    robtillaart/CRC@^1.0.2
```

### Basic Usage

Include the library and define your data structure:

```cpp
#include <Arduino.h>
#include <EEPROM.h>

#define USE_COLOR_TERMINAL
#define OVERRIDE_EEPROM_MAX_WRITES_PER_BLOCK 2 // For testing purposes
#include <Utilify/EEPROM/EEPROMDataStorage.h>

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
```

## EEPROM Memory Layout

The EEPROM memory is organized as follows:

| Section                      | Size (bytes) |
|:------------------------------|:------------:|
| Application Signature         | 2 bytes      |
| Start Address of Data Block    | 2 bytes      |
| Write Counter                 | 2 bytes      |
| CRC16 Checksum of User Data    | 2 bytes      |
| User Data                     | `sizeof(DataType)` bytes |

The library handles block switching automatically when the maximum number of writes per block is reached.

## Example Output

Typical serial monitor output:

```
Data block size: 24
App signature: 0x0666
Start data address: 0x001c
User data CRC address: 0x001e
User data start address: 0x0020
Write counter: 2
User data CRC: 0x162b

EEPROM Content:
Dump capacity: 1024
      00  01  02  03  04  05  06  07  08  09  0a  0b  0c  0d  0e  0f
0000  66  06  1c  00  02  00  2b  16  34  12  21  43  43  68  6f  63  |  f     + 4 !CChoc  |
0010  6f  6c  61  74  69  6e  65  21  00  00  00  00  02  00  2b  16  |  olatine!      +   |
0020  34  12  21  43  43  68  6f  63  6f  6c  61  74  69  6e  65  21  |  4 !CChocolatine!  |
0030  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0040  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0050  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0060  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0070  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0080  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0090  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
00a0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
00b0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
00c0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
00d0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
00e0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
00f0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0100  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0110  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0120  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0130  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0140  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0150  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0160  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0170  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0180  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0190  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
01a0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
01b0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
01c0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
01d0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
01e0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
01f0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0200  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0210  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0220  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0230  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0240  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0250  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0260  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0270  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0280  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0290  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
02a0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
02b0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
02c0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
02d0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
02e0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
02f0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0300  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0310  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0320  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0330  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0340  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0350  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0360  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0370  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0380  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
0390  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
03a0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
03b0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
03c0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
03d0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
03e0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |
03f0  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  00  |                    |

Final load successful.
Data content:
a = 0x1234
b = 0x4321
c = Chocolatine!
```

## Dependencies

- **Arduino Core** (for `Serial` and `EEPROM`)
- **CRC16 Library** (for checksum calculation)
- **Utilify**: [https://github.com/PiFou86/Utilify.git](https://github.com/PiFou86/Utilify.git)

## Author

**Pierre-François Léon**

[![LinkedIn](https://img.shields.io/badge/LinkedIn-Profile-blue?logo=linkedin&style=for-the-badge)](https://www.linkedin.com/in/pierrefrancoisleon/)
[![GitHub profile](https://img.shields.io/badge/GitHub-Profile-blue?logo=github&style=for-the-badge)](https://github.com/PiFou86)
