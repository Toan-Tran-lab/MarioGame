#pragma once
#include "core/core.h"
#include "raylib.h"
#include <vector>
#include <string>

class GameplayState;

class PauseMenuState : public IGameState {
private:
    struct MenuEntry {
        std::string label;
        bool enabled;
    };
    
    std::vector<MenuEntry> entries;
    int selectedIndex;
    float timeAccum;
    Vector2 lastMousePos = { -1.0f, -1.0f };
    
    GameplayState* parentState;

    Rectangle GetItemRect(int index, float sw, float sh) const;

public:
    PauseMenuState(GameplayState* parent);
    
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Cleanup() override;
};
