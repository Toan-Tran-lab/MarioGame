#include "LevelCompleteState.h"
#include "GameplayState/GameplayState.h"
#include "Global/Global.h"
#include <cmath>
#include <string>

LevelCompleteState::LevelCompleteState(GameplayState* parent, int levelId, int characterId, int score, float timeLeft)
    : parentState(parent), levelId(levelId), characterId(characterId), score(score), timeLeft(timeLeft), timeAccum(0.0f) {
}

void LevelCompleteState::Initialize() {
}

void LevelCompleteState::Update(float deltaTime) {
    timeAccum += deltaTime;

    // Press ENTER to go back to level select
    if (IsKeyPressed(KEY_ENTER)) {
        Global::gameStateManager->PopState(); // Pop LevelCompleteState
        Global::gameStateManager->PopState(); // Pop GameplayState
    }
}

void LevelCompleteState::Draw() {
    // Draw the gameplay behind the level complete screen
    if (parentState) {
        parentState->Draw();
    }
    
    // Dark overlay
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();
    DrawRectangle(0, 0, (int)sw, (int)sh, Color{0, 0, 0, 150});
    
    // Draw "COURSE CLEAR!" text
    int titleSize = (int)(sh * 0.1f);
    int titleW = MeasureText("COURSE CLEAR!", titleSize);
    DrawText("COURSE CLEAR!", (int)(sw - titleW) / 2, (int)(sh * 0.2f), titleSize, WHITE);
    
    // Draw Score
    int scoreSize = (int)(sh * 0.05f);
    std::string scoreText = "SCORE: " + std::to_string(score);
    int scoreW = MeasureText(scoreText.c_str(), scoreSize);
    DrawText(scoreText.c_str(), (int)(sw - scoreW) / 2, (int)(sh * 0.4f), scoreSize, WHITE);

    // Draw Time
    std::string timeText = "TIME: " + std::to_string((int)timeLeft);
    int timeW = MeasureText(timeText.c_str(), scoreSize);
    DrawText(timeText.c_str(), (int)(sw - timeW) / 2, (int)(sh * 0.5f), scoreSize, WHITE);

    // Draw Press Enter prompt (blinking)
    if (fmod(timeAccum, 1.0f) < 0.5f) {
        std::string promptText = "Press ENTER to continue";
        int promptSize = (int)(sh * 0.04f);
        int promptW = MeasureText(promptText.c_str(), promptSize);
        DrawText(promptText.c_str(), (int)(sw - promptW) / 2, (int)(sh * 0.8f), promptSize, WHITE);
    }
}

void LevelCompleteState::Cleanup() {
}
