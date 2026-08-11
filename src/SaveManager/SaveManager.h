#pragma once
#include "raylib.h"
#include <string>

struct SaveData {
    int levelId = 1;
    float playerX = 0.0f;
    float playerY = 0.0f;
    int score = 0;
    float timeLeft = 300.0f;
    std::string timestamp; // For UI display
};

class SaveManager {
private:
    static std::string GetSavePath(const std::string& slotName);
    static std::string GetCurrentTimestamp();

public:
    // slotName can be "auto_save", "slot1", "slot2", "slot3"
    static bool SaveGame(const std::string& slotName, const SaveData& data);
    static bool LoadGame(const std::string& slotName, SaveData& outData);
    static bool SaveExists(const std::string& slotName);
};
