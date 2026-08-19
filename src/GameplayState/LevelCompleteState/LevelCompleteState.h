#pragma once
#include "core/core.h"
#include <string>

class GameplayState;

class LevelCompleteState : public IGameState {
private:
    GameplayState* parentState;
    int levelId;
    int characterId;
    int score;
    float timeLeft;
    float timeAccum;

public:
    LevelCompleteState(GameplayState* parent, int levelId, int characterId, int score, float timeLeft);

    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Cleanup() override;
};
