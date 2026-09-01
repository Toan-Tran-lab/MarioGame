#include "LevelCompleteState.h"
#include "GameplayState/GameplayState.h"
#include "Global/Global.h"
#include "Level/Level.h"
#include "TextureManager/TextureManager.h"
#include "ui/UIUtils.h"
#include <cmath>
#include <string>

LevelCompleteState::LevelCompleteState(GameplayState* parent, int levelId, int characterId, int score, float timeLeft)
    : parentState(parent), levelId(levelId), characterId(characterId), score(score), timeLeft(timeLeft), timeAccum(0.0f) {
}

void LevelCompleteState::Initialize() {
    UIUtils::InitMenuBackground();
    TextureManager::Load("title_logo", "assets/textures/group6mario.png");
    selectedButton = 0;
}

Rectangle LevelCompleteState::GetButtonRect(int index, float sw, float sh) const {
    float cardW = sw * 0.6f;
    float cardH = sh * 0.8f;
    float cardY = sh * 0.1f;
    
    float itemW = cardW * 0.7f;
    float itemH = cardH * 0.08f;
    float itemX = (sw - itemW) * 0.5f;
    
    float startY = cardY + cardH * 0.72f;
    float gap = cardH * 0.03f;
    return { itemX, startY + index * (itemH + gap), itemW, itemH };
}

void LevelCompleteState::Update(float deltaTime) {
    timeAccum += deltaTime;
    UIUtils::UpdateMenuBackground(deltaTime);

    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();
    Vector2 mouse = GetMousePosition();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    // Keyboard navigation
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) ||
        IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S)) {
        selectedButton = 1 - selectedButton;
    }

    // Mouse hover
    if (mouse.x != lastMousePos.x || mouse.y != lastMousePos.y) {
        lastMousePos = mouse;
        for (int i = 0; i < 2; i++) {
            if (CheckCollisionPointRec(mouse, GetButtonRect(i, sw, sh)))
                selectedButton = i;
        }
    }

    // Confirm: ENTER or click on selected button
    bool confirm = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
        (clicked && CheckCollisionPointRec(mouse, GetButtonRect(selectedButton, sw, sh)));

    if (confirm) {
        if (selectedButton == 0) {
            // NEXT LEVEL — reload the parent GameplayState with the next level
            int nextLevelNumber = levelId + 1;
            if (nextLevelNumber <= Level::GetTotalLevels() && parentState) {
                Global::gameStateManager->PopState(); // Pop LevelCompleteState (this)
                parentState->ResetForNewLevel();
                parentState->SetLevel(Level::GetLevel(nextLevelNumber));
                parentState->Cleanup();
                parentState->Initialize();
            } else {
                // No next level — return to main menu
                Global::gameStateManager->PopState(); // Pop LevelCompleteState
                Global::gameStateManager->PopState(); // Pop GameplayState
            }
        } else {
            // RETURN TO MENU
            Global::gameStateManager->PopState(); // Pop LevelCompleteState
            Global::gameStateManager->PopState(); // Pop GameplayState
        }
    }

    // ESC always returns to menu
    if (IsKeyPressed(KEY_ESCAPE)) {
        Global::gameStateManager->PopState(); // Pop LevelCompleteState
        Global::gameStateManager->PopState(); // Pop GameplayState
    }
}

void LevelCompleteState::DrawButtons(float sw, float sh) const {
    const char* labels[2] = { "NEXT LEVEL", "RETURN TO MENU" };
    int fontSize = (int)(sh * 0.04f);

    for (int i = 0; i < 2; i++) {
        Rectangle r = GetButtonRect(i, sw, sh);
        bool isSelected = (i == selectedButton);
        Color textColor = isSelected ? YELLOW : WHITE;

        // Draw selection arrow
        if (isSelected) {
            int iconX = (int)(r.x - MeasureText(">", fontSize) - (int)(sw * 0.015f));
            int iconY = (int)(r.y + (r.height - fontSize) * 0.5f);
            UIUtils::DrawBlinkingText(">", iconX, iconY, fontSize, YELLOW, timeAccum);
        }

        DrawText(labels[i], (int)r.x, (int)(r.y + (r.height - fontSize) * 0.5f), fontSize, textColor);
    }
}

void LevelCompleteState::Draw() {
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();

    // Animated level background (same as main menu)
    UIUtils::DrawMenuBackground(sw, sh);

    // Panel card
    float cardW = sw * 0.6f;
    float cardH = sh * 0.8f;
    float cardX = (sw - cardW) * 0.5f;
    float cardY = sh * 0.1f;
    DrawRectangleRounded({ cardX, cardY, cardW, cardH }, 0.08f, 12, Color{ 30, 18, 45, 220 });
    DrawRectangleRoundedLines({ cardX, cardY, cardW, cardH }, 0.08f, 12, Color{ 180, 150, 220, 200 });

    // Title: "LEVEL COMPLETE" — styled like the main menu logo
    const char* titleText = "LEVEL COMPLETE";
    int titleSize = (int)(sh * 0.065f); // slightly smaller to fit better
    Vector2 titleMeasure = MeasureTextEx(Global::titleFont, titleText, (float)titleSize, 1.0f);
    float titleX = (sw - titleMeasure.x) / 2.0f;
    float titleY = cardY + cardH * 0.05f;

    // Thick outline: draw 8 offset copies in dark red/brown
    int outlineRadius = (int)(titleSize * 0.09f);
    Color outlineColor = Color{ 140, 20, 20, 255 };
    for (int ox = -outlineRadius; ox <= outlineRadius; ox += outlineRadius) {
        for (int oy = -outlineRadius; oy <= outlineRadius; oy += outlineRadius) {
            if (ox == 0 && oy == 0) continue;
            DrawTextEx(Global::titleFont, titleText, { titleX + ox, titleY + oy }, (float)titleSize, 1.0f, outlineColor);
        }
    }
    // White fill on top
    DrawTextEx(Global::titleFont, titleText, { titleX, titleY }, (float)titleSize, 1.0f, WHITE);

    // Divider line 1
    float div1Y = cardY + cardH * 0.22f;
    DrawLineEx({ cardX + cardW * 0.1f, div1Y }, { cardX + cardW * 0.9f, div1Y }, 2.0f, Color{ 180, 150, 220, 120 });

    // Stats
    int statSize = (int)(cardH * 0.06f);
    int statGap  = (int)(cardH * 0.12f);
    int statY    = (int)(cardY + cardH * 0.28f);

    std::string levelText = "LEVEL  " + std::to_string(levelId);
    std::string scoreText = "SCORE  " + std::to_string(score);
    std::string timeText  = "TIME   " + std::to_string((int)timeLeft);

    UIUtils::DrawCenteredText(levelText.c_str(), statY,             statSize, WHITE,  (int)sw);
    UIUtils::DrawCenteredText(scoreText.c_str(), statY + statGap,   statSize, WHITE,  (int)sw);
    UIUtils::DrawCenteredText(timeText.c_str(),  statY + statGap*2, statSize, WHITE,  (int)sw);

    // Divider line 2
    float div2Y = cardY + cardH * 0.65f;
    DrawLineEx({ cardX + cardW * 0.1f, div2Y }, { cardX + cardW * 0.9f, div2Y }, 2.0f, Color{ 180, 150, 220, 120 });

    // Buttons
    DrawButtons(sw, sh);

    // Key hint bar at the bottom of the card
    int hintSize = (int)(sh * 0.025f);
    float hintX = cardX + cardW * 0.1f;
    float hintY = cardY + cardH - sh * 0.05f;
    
    // Instead of using spacing arg, we manually calculate the next X pos to avoid overlap
    int enterW = MeasureText("ENTER", hintSize);
    int selW = MeasureText("Select", hintSize);
    UIUtils::DrawKeyPrompt("ENTER", "Select", hintX, hintY, hintSize, 0);
    hintX += enterW + selW + 25; // Add extra margin
    UIUtils::DrawKeyPrompt("ESC", "Menu",     hintX, hintY, hintSize, 0);
}

void LevelCompleteState::Cleanup() {
}
