#pragma once
#include <Arduino.h>
#include <map>
#include <SD.h>
#include <SPI.h>
#include <Bluepad32.h>

class GamepadConfig {
public:
    GamepadConfig();
    bool initSD(int clk, int miso, int mosi, int cs);
    void loadConfig(String macAddress);
    
    // Return true if the specified virtual button is currently pressed
    bool isPressed(const String& btnName, uint64_t currentGamepadMask);
    
private:
    std::map<String, uint64_t> mappings;
    bool sdInitialized;
};

extern GamepadConfig g_gamepadConfig;
