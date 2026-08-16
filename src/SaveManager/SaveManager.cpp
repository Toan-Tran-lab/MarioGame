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

    j["isSandboxMode"] = data.isSandboxMode;
    if (data.isSandboxMode) {
        json jGrid = json::array();
        for (int r = 0; r < (int)data.sandboxGrid.size(); ++r) {
            for (int c = 0; c < (int)data.sandboxGrid[r].size(); ++c) {
                const auto& cell = data.sandboxGrid[r][c];
                if (cell.type != 0) {
                    json jCell;
                    jCell["r"] = r;
                    jCell["c"] = c;
                    jCell["type"] = cell.type;
                    jCell["texKey"] = cell.texKey;
                    jCell["srcRect"] = { cell.srcRect.x, cell.srcRect.y, cell.srcRect.width, cell.srcRect.height };
                    jCell["isSolid"] = cell.isSolid;
                    jGrid.push_back(jCell);
                }
            }
        }
        j["sandboxGrid"] = jGrid;
        j["sandboxRows"] = (int)data.sandboxGrid.size();
        j["sandboxCols"] = (int)(data.sandboxGrid.empty() ? 0 : data.sandboxGrid[0].size());
    }

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

        outData.isSandboxMode = j.value("isSandboxMode", false);
        if (outData.isSandboxMode) {
            int rows = j.value("sandboxRows", 200);
            int cols = j.value("sandboxCols", 200);
            outData.sandboxGrid.clear();
            outData.sandboxGrid.resize(rows, std::vector<SandboxCellData>(cols, {0, "", {0,0,0,0}, false}));

            for (const auto& jCell : j["sandboxGrid"]) {
                int r = jCell["r"];
                int c = jCell["c"];
                if (r >= 0 && r < rows && c >= 0 && c < cols) {
                    outData.sandboxGrid[r][c].type = jCell["type"];
                    outData.sandboxGrid[r][c].texKey = jCell["texKey"];
                    std::vector<float> rectVal = jCell["srcRect"];
                    if (rectVal.size() == 4) {
                        outData.sandboxGrid[r][c].srcRect = { rectVal[0], rectVal[1], rectVal[2], rectVal[3] };
                    }
                    outData.sandboxGrid[r][c].isSolid = jCell.value("isSolid", true);
                }
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool SaveManager::SaveExists(const std::string& slotName) {
    std::ifstream file(GetSavePath(slotName));
    return file.is_open();
}
