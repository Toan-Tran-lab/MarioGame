#pragma once
#include "raylib.h"
#include "core/core.h"

// Character select screen (Mario / Luigi) that launches the sandbox gameplay state
class CharacterSelectState : public IGameState {
private:
    float timeAccum = 0.0f;

public:
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Cleanup() override;
};
