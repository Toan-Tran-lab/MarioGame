#include "CharacterSelectState.h"
#include "Global/Global.h"
#include "ui/UIUtils.h"
#include "GameplayState/GameplayState.h"
#include <cmath>
#include <memory>

namespace {
    int selectedCharacter = 0; // 0 = Mario, 1 = Luigi
}

void CharacterSelectState::SetLevel(const Level& level) {
    selectedLevel = level;
}

void CharacterSelectState::Initialize() {
    timeAccum = 0.0f;
    selectedCharacter = 0;
}

// LEFT/RIGHT to switch character, ENTER to confirm and start the sandbox,
// ESC to return to the main menu.
void CharacterSelectState::Update(float deltaTime) {
    timeAccum += deltaTime;

    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
        selectedCharacter = !selectedCharacter;
    }

    if (IsKeyPressed(Global::keys.select) || IsKeyPressed(KEY_ENTER)) {
        auto gameplay = std::make_unique<GameplayState>();
        gameplay->SetLevel(selectedLevel);
        Global::gameStateManager->PushState(std::move(gameplay));
    }

    if (IsKeyPressed(Global::keys.back) || IsKeyPressed(KEY_ESCAPE)) {
        Global::gameStateManager->PopState();
    }
    
    // Mouse hover and click logic
    Vector2 mousePos = GetMousePosition();
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();
    float panelW = sw * 0.3f, panelH = sh * 0.4f;
    float panelY = sh * 0.42f;
    float marioX = sw * 0.5f - sw * 0.32f;
    float luigiX = sw * 0.5f + sw * 0.02f;
    Rectangle marioRect = { marioX, panelY, panelW, panelH };
    Rectangle luigiRect = { luigiX, panelY, panelW, panelH };
    
    if (mousePos.x != lastMousePos.x || mousePos.y != lastMousePos.y) {
        lastMousePos = mousePos;
        if (CheckCollisionPointRec(mousePos, marioRect)) selectedCharacter = 0;
        else if (CheckCollisionPointRec(mousePos, luigiRect)) selectedCharacter = 1;
    }
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(mousePos, marioRect) || CheckCollisionPointRec(mousePos, luigiRect)) {
            auto gameplay = std::make_unique<GameplayState>();
            gameplay->SetLevel(selectedLevel);
            Global::gameStateManager->PushState(std::move(gameplay));
        }
    }
}

// Draw the character select screen with Mario and Luigi panels.
void CharacterSelectState::Draw() {
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();

    ClearBackground(Color{ 60, 40, 80, 255 });

    int titleSize = (int)(sh * 0.06f);
    const char* title = "SELECT CHARACTER";
    int titleW = MeasureText(title, titleSize);
    DrawText(title, (int)((sw - titleW) * 0.5f), (int)(sh * 0.3f), titleSize, WHITE);

    float panelW = sw * 0.3f, panelH = sh * 0.4f;
    float panelY = sh * 0.42f;
    float marioX = sw * 0.5f - sw * 0.32f;
    float luigiX = sw * 0.5f + sw * 0.02f;

    Rectangle marioRect = { marioX, panelY, panelW, panelH };
    Rectangle luigiRect = { luigiX, panelY, panelW, panelH };

    DrawRectangleRec(marioRect, Color{ 40, 25, 55, 255 });
    DrawRectangleRec(luigiRect, Color{ 40, 25, 55, 255 });

    int nameSize = (int)(sh * 0.05f);
    UIUtils::DrawCenteredText("MARIO", (int)(marioRect.y + marioRect.height * 0.5f), nameSize, RED, (int)sw);
    UIUtils::DrawCenteredText("LUIGI", (int)(luigiRect.y + luigiRect.height * 0.5f), nameSize, GREEN, (int)sw);

    Rectangle selectedRect = (selectedCharacter == 0) ? marioRect : luigiRect;
    DrawRectangleLinesEx(selectedRect, 4, YELLOW);

    int cursorX = (int)(selectedRect.x - MeasureText(">", nameSize) - sw * 0.015f);
    int cursorY = (int)(selectedRect.y + (selectedRect.height - nameSize) * 0.5f);
    UIUtils::DrawBlinkingText(">", cursorX, cursorY, nameSize, YELLOW, timeAccum);

    int barY = (int)(sh * 0.92f);
    DrawLine(0, barY, (int)sw, barY, Color{ 100, 80, 120, 255 });
    int barFontSize = (int)(sh * 0.025f);
    int spacing = (int)(sw * 0.08f);
    float x = sw - spacing * 3;

    UIUtils::DrawKeyPrompt("L/R", "SWITCH", x, barY + 5, barFontSize, spacing);
    UIUtils::DrawKeyPrompt("ENTER", "SELECT", x, barY + 5, barFontSize, spacing);
    UIUtils::DrawKeyPrompt("ESC", "BACK", x, barY + 5, barFontSize, spacing);
}

void CharacterSelectState::Cleanup() {}
