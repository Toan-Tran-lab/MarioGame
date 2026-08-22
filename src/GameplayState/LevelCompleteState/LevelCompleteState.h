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
    int selectedButton = 0; // 0 = NEXT LEVEL, 1 = RETURN TO MENU
    Vector2 lastMousePos = {-1, -1};

    Rectangle GetButtonRect(int index, float sw, float sh) const;
    void DrawButtons(float sw, float sh) const;

public:
    LevelCompleteState(GameplayState* parent, int levelId, int characterId, int score, float timeLeft);

    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Cleanup() override;
};
