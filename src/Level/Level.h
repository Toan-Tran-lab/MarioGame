#pragma once
#include <string>
#include "AudioManager/AudioManager.h"

// Represents a game level with its metadata and LDtk mapping
class Level {
private:
    int levelNumber;            // 1, 2, 3 (player-facing)
    std::string ldtkLevelId;    // LDtk identifier (e.g. "Level_0")
    std::string displayName;    // Display name (e.g. "Level 1 - Grassland")

public:
    Level();
    Level(int number, const std::string& ldtkId, const std::string& name);

    int GetLevelNumber() const;
    const std::string& GetLdtkLevelId() const;
    const std::string& GetDisplayName() const;
    // Returns the AudioKey constant for this level's BGM
    const char* GetBGMKey() const;

    // Predefined levels
    static Level GetLevel(int number);
    static int GetTotalLevels();
};
