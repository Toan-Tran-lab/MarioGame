#pragma once
#include "core/core.h"
#include "raylib.h"
#include <vector>
#include <string>

class GameplayState;

class SaveLoadState : public IGameState {
public:
    enum class Mode { Save, Load };

private:
    Mode mode;
    GameplayState* gameplayState; // Used in Save mode to extract data

    struct SlotEntry {
        std::string name;
        std::string filename;
        std::string info;
        bool exists;
    };
    
    std::vector<SlotEntry> slots;
    int selectedIndex;
    float timeAccum;
    Vector2 lastMousePos = { -1.0f, -1.0f };

    Rectangle GetItemRect(int index, float sw, float sh) const;
    void RefreshSlots();

public:
    SaveLoadState(Mode m, GameplayState* gs = nullptr);
    
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Cleanup() override;
};
