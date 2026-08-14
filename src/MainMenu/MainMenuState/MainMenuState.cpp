#include "MainMenuState.h"
#include "MainMenu/LevelSelectState/LevelSelectState.h"
#include "MainMenu/SettingsState/SettingsState.h"
#include "MainMenu/CharacterSelectState/CharacterSelectState.h"
#include "MainMenu/SaveLoadState/SaveLoadState.h"
#include "Global/Global.h"
#include "TextureManager/TextureManager.h"
#include "ui/UIUtils.h"
#include <cmath>
#include <memory>

void MainMenuState::Initialize() {
    entries = { {"NEW GAME", true}, {"CONTINUE", false}, {"SETTING", true} };
    selectedIndex = 0;
    timeAccum = 0.0f;
    showExitPrompt = false;
    exitChoice = 0;

    UIUtils::InitMenuBackground();
}

Rectangle MainMenuState::GetItemRect(int index, float sw, float sh) const {
    float itemW = sw * 0.3f, itemH = sh * 0.06f;
    float itemX = (sw - itemW) * 0.5f;
    return { itemX, sh * 0.55f + index * (itemH + sh * 0.03f), itemW, itemH };
}

void MainMenuState::Update(float deltaTime) {
    timeAccum += deltaTime;
    UIUtils::UpdateMenuBackground(deltaTime);
    
    float sw = (float)GetScreenWidth(), sh = (float)GetScreenHeight();
    entries[1].enabled = Global::hasSaveGame;

    Vector2 mouse = GetMousePosition();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    if (showExitPrompt) {
        UpdateExitPrompt(sw, sh, mouse, clicked);
        return;
    }

    UpdateMenuSelection(sw, sh, mouse);

    if (IsKeyPressed(KEY_ESCAPE)) {
        showExitPrompt = true;
    }

    if (clicked || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (entries[selectedIndex].enabled) HandleMenuAction();
    }
}

void MainMenuState::UpdateExitPrompt(float sw, float sh, Vector2 mouse, bool clicked) {
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) {
        exitChoice = !exitChoice;
    }

    bool confirm = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || clicked;
    if (confirm) {
        if (exitChoice == 0) Global::shouldExit = true;
        else { showExitPrompt = false; exitChoice = 0; }
    }
    if (IsKeyPressed(KEY_ESCAPE)) { showExitPrompt = false; exitChoice = 0; }

    if (clicked) {
        float optY = sh * 0.45f, optW = sw * 0.1f, optH = sh * 0.06f;
        if (CheckCollisionPointRec(mouse, { sw * 0.35f, optY, optW, optH })) Global::shouldExit = true;
        else if (CheckCollisionPointRec(mouse, { sw * 0.55f, optY, optW, optH })) {
            showExitPrompt = false; exitChoice = 0;
        }
    }
}

void MainMenuState::UpdateMenuSelection(float sw, float sh, Vector2 mouse) {
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S)) {
        int dir = (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) ? 1 : -1;
        int count = (int)entries.size();
        for (int i = 0; i < count; i++) {
            selectedIndex = (selectedIndex + dir + count) % count;
            if (entries[selectedIndex].enabled) break;
        }
    }

    if (mouse.x != lastMousePos.x || mouse.y != lastMousePos.y) {
        lastMousePos = mouse;
        for (size_t i = 0; i < entries.size(); i++) {
            if (!entries[i].enabled) continue;
            if (CheckCollisionPointRec(mouse, GetItemRect((int)i, sw, sh))) {
                selectedIndex = (int)i;
            }
        }
    }
}

void MainMenuState::HandleMenuAction() {
    switch (selectedIndex) {
        case 0: Global::gameStateManager->PushState(std::make_unique<LevelSelectState>()); break;
        case 1:
            if (Global::hasSaveGame) {
                Global::gameStateManager->PushState(std::make_unique<SaveLoadState>(SaveLoadState::Mode::Load));
            }
            break;
        case 2: Global::gameStateManager->PushState(std::make_unique<SettingsState>()); break;
    }
}

void MainMenuState::Draw() {
    float sw = (float)GetScreenWidth(), sh = (float)GetScreenHeight();
    ClearBackground(Color{ 60, 40, 80, 255 });
    
    UIUtils::DrawMenuBackground(sw, sh);
    DrawMenuEntries(sw, sh);
    if (showExitPrompt) DrawExitPromptDialog(sw, sh);
}



void MainMenuState::DrawMenuEntries(float sw, float sh) const {
    int menuFontSize = (int)(sh * 0.04f);
    for (size_t i = 0; i < entries.size(); i++) {
        Rectangle r = GetItemRect((int)i, sw, sh);
        bool isSelected = ((int)i == selectedIndex);
        Color textColor = !entries[i].enabled ? Color{ 128, 128, 128, 128 } : (isSelected ? YELLOW : WHITE);

        if (isSelected && entries[i].enabled) {
            int iconX = (int)(r.x - MeasureText(">", menuFontSize) - sw * 0.015f);
            int iconY = (int)(r.y + (r.height - menuFontSize) * 0.5f);
            UIUtils::DrawBlinkingText(">", iconX, iconY, menuFontSize, YELLOW, timeAccum);
        }
        DrawText(entries[i].label.c_str(), (int)r.x, (int)(r.y + (r.height - menuFontSize) * 0.5f), menuFontSize, textColor);
    }
}

void MainMenuState::DrawExitPromptDialog(float sw, float sh) const {
    DrawRectangle(0, 0, (int)sw, (int)sh, Color{ 0, 0, 0, 180 });

    float boxW = sw * 0.4f, boxH = sh * 0.2f, boxX = (sw - boxW) * 0.5f, boxY = sh * 0.35f;
    DrawRectangleRec({ boxX, boxY, boxW, boxH }, Color{ 40, 25, 55, 255 });
    DrawRectangleLinesEx({ boxX, boxY, boxW, boxH }, 2, Color{ 180, 160, 200, 255 });

    int promptFontSize = (int)(sh * 0.045f);
    UIUtils::DrawCenteredText("QUIT GAME?", (int)(boxY + sh * 0.03f), promptFontSize, WHITE, (int)sw);

    int optFontSize = (int)(sh * 0.035f);
    float optY = boxY + boxH * 0.55f;
    float yesX = sw * 0.35f, noX = sw * 0.55f;

    Color yesColor = (exitChoice == 0) ? YELLOW : Color{ 200, 200, 200, 255 };
    Color noColor  = (exitChoice == 1) ? YELLOW : Color{ 200, 200, 200, 255 };
    int iconY = (int)(optY + (sh*0.06f - optFontSize)*0.5f);
    
    if (exitChoice == 0) {
        int iconX = (int)(yesX - MeasureText(">", optFontSize) - 8);
        UIUtils::DrawBlinkingText(">", iconX, iconY, optFontSize, YELLOW, timeAccum);
    }
    DrawText("YES", (int)yesX, iconY, optFontSize, yesColor);

    if (exitChoice == 1) {
        int iconX = (int)(noX - MeasureText(">", optFontSize) - 8);
        UIUtils::DrawBlinkingText(">", iconX, iconY, optFontSize, YELLOW, timeAccum);
    }
    DrawText("NO", (int)noX, iconY, optFontSize, noColor);
}

void MainMenuState::Cleanup() {
    UIUtils::CleanupMenuBackground();
}
