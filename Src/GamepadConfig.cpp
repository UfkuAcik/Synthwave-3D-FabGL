#include "GamepadConfig.h"

GamepadConfig g_gamepadConfig;

GamepadConfig::GamepadConfig() : sdInitialized(false) {}

bool GamepadConfig::initSD(int clk, int miso, int mosi, int cs) {
    SPI.begin(clk, miso, mosi, cs);
    if (!SD.begin(cs)) {
        Serial.println("SD Card Mount Failed");
        return false;
    }
    sdInitialized = true;
    Serial.println("SD Card Initialized");
    return true;
}

void GamepadConfig::loadConfig(String macAddress) {
    if (!sdInitialized) return;
    
    // Clear old mappings
    mappings.clear();
    
    // Default fallback mappings just in case the file is missing or gamepad not configured
    mappings["A"] = (1ULL << 0);
    mappings["B"] = (1ULL << 1);
    mappings["X"] = (1ULL << 2);
    mappings["Y"] = (1ULL << 3);
    mappings["DPAD_UP"] = (1ULL << 16);
    mappings["DPAD_DOWN"] = (1ULL << 17);
    mappings["DPAD_LEFT"] = (1ULL << 18);
    mappings["DPAD_RIGHT"] = (1ULL << 19);
    mappings["L1"] = (1ULL << 4);
    mappings["R1"] = (1ULL << 5);
    mappings["L2"] = (1ULL << 6);
    mappings["R2"] = (1ULL << 7);
    mappings["START"] = (1ULL << 13);
    
    File file = SD.open("/Button_Config/mappings.cfg", FILE_READ);
    if (!file) {
        Serial.println("No mappings.cfg found. Using defaults.");
        return;
    }

    String targetHeader = "[TYP_0000_MAC_" + macAddress + "]";
    // We should also check for standard VID/PID but the tester saves as MAC if they are 0000.
    // The easiest is just exact match to whatever the Gamepad tester writes.
    // Actually, Gamepad tester writes [TYP_XXXX_MAC_XXXX] or whatever.
    // Let's just find the exact MAC in the file.
    
    bool readingMyController = false;

    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        
        if (line.startsWith("[")) {
            // Check if it contains the MAC address
            if (line.indexOf(macAddress) > 0) {
                readingMyController = true;
                Serial.println("Found config section for this gamepad: " + line);
            } else {
                readingMyController = false;
            }
        } else if (readingMyController) {
            int eqPos = line.indexOf('=');
            if (eqPos > 0) {
                String btnName = line.substring(0, eqPos);
                String maskStr = line.substring(eqPos + 1);
                
                uint64_t mask = strtoull(maskStr.c_str(), NULL, 10);
                mappings[btnName] = mask;
                Serial.printf("Mapped %s to %llu\n", btnName.c_str(), mask);
            }
        }
    }
    file.close();
}

bool GamepadConfig::isPressed(const String& btnName, uint64_t currentGamepadMask) {
    if (mappings.find(btnName) != mappings.end()) {
        uint64_t mask = mappings[btnName];
        return (currentGamepadMask & mask) != 0;
    }
    return false;
}
