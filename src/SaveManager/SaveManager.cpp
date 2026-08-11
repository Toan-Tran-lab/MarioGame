#include "SaveManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>

#if defined(_WIN32)
#include <direct.h>
#define MKDIR(dir) _mkdir(dir)
#else
#include <sys/stat.h>
#define MKDIR(dir) mkdir(dir, 0777)
#endif

using json = nlohmann::json;

std::string SaveManager::GetSavePath(const std::string& slotName) {
    return "saves/" + slotName + ".json";
}

std::string SaveManager::GetCurrentTimestamp() {
    std::time_t t = std::time(nullptr);
    std::tm* tm = std::localtime(&t);
    std::ostringstream ss;
    ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

bool SaveManager::SaveGame(const std::string& slotName, const SaveData& data) {
    MKDIR("saves"); // Ensure directory exists

    json j;
    j["levelId"] = data.levelId;
    j["playerX"] = data.playerX;
    j["playerY"] = data.playerY;
    j["score"] = data.score;
    j["timeLeft"] = data.timeLeft;
    j["timestamp"] = GetCurrentTimestamp();

    std::ofstream file(GetSavePath(slotName));
    if (file.is_open()) {
        file << j.dump(4);
        return true;
    }
    return false;
}

bool SaveManager::LoadGame(const std::string& slotName, SaveData& outData) {
    std::ifstream file(GetSavePath(slotName));
    if (!file.is_open()) return false;

    try {
        json j;
        file >> j;
        outData.levelId = j.value("levelId", 1);
        outData.playerX = j.value("playerX", 0.0f);
        outData.playerY = j.value("playerY", 0.0f);
        outData.score = j.value("score", 0);
        outData.timeLeft = j.value("timeLeft", 300.0f);
        outData.timestamp = j.value("timestamp", "");
        return true;
    } catch (...) {
        return false;
    }
}

bool SaveManager::SaveExists(const std::string& slotName) {
    std::ifstream file(GetSavePath(slotName));
    return file.is_open();
}
