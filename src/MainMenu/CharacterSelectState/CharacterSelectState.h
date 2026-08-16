#pragma once
#include "raylib.h"
#include "core/core.h"
#include "Level/Level.h"
#include "world/TileMap.h"
#include <vector>

// Character select screen (Mario / Luigi) that launches the gameplay state with the chosen level
class CharacterSelectState : public IGameState {
private:
    float timeAccum = 0.0f;
    Level selectedLevel;
    Vector2 lastMousePos = { -1.0f, -1.0f };

    bool isSandboxMode_ = false;
    std::vector<std::vector<SandboxCellData>> sandboxGrid_;

public:
    void SetLevel(const Level& level);
    void SetSandboxMode(const std::vector<std::vector<SandboxCellData>>& grid);
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Cleanup() override;
};
