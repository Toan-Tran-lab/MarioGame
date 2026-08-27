#include "Level.h"

Level::Level()
    : levelNumber(0), ldtkLevelId(""), displayName("") {}

Level::Level(int number, const std::string& ldtkId, const std::string& name)
    : levelNumber(number), ldtkLevelId(ldtkId), displayName(name) {}

int Level::GetLevelNumber() const { return levelNumber; }
const std::string& Level::GetLdtkLevelId() const { return ldtkLevelId; }
const std::string& Level::GetDisplayName() const { return displayName; }

// Central definition of all levels — add new levels here
Level Level::GetLevel(int number) {
    switch (number) {
        case 1:  return Level(1, "Level_0", "Level 1 - Grassland");
        case 2:  return Level(2, "Level_1", "Level 2 - Underground");
        case 3:  return Level(3, "Level_2", "Level 3 - Castle");
        default: return Level(1, "Level_0", "Level 1 - Grassland");
    }
}

int Level::GetTotalLevels() { return 3; }

const char* Level::GetBGMKey() const {
    switch (levelNumber) {
        case 1:  return AudioKey::BGM_LEVEL_1;
        case 2:  return AudioKey::BGM_LEVEL_2;
        case 3:  return AudioKey::BGM_LEVEL_3;
        default: return AudioKey::BGM_LEVEL_1;
    }
}
