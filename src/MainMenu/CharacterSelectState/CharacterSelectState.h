#pragma once
#include "raylib.h"
#include "core/core.h"
#include "Level/Level.h"

// Character select screen (Mario / Luigi) that launches the gameplay state with the chosen level
class CharacterSelectState : public IGameState {
private:
    float timeAccum = 0.0f;
    Level selectedLevel;
    Vector2 lastMousePos = { -1.0f, -1.0f };

public:
    void SetLevel(const Level& level);
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Cleanup() override;
};
