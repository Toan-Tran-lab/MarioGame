#include "PauseMenuState.h"
#include "MainMenu/SettingsState/SettingsState.h"
#include "MainMenu/SaveLoadState/SaveLoadState.h"
#include "GameplayState/GameplayState.h"
#include "Global/Global.h"
#include <cmath>

PauseMenuState::PauseMenuState(GameplayState* parent) 
    : parentState(parent), selectedIndex(0), timeAccum(0.0f) {}

void PauseMenuState::Initialize() {
    entries = {
        {"RESUME", true},
        {"SAVE GAME", true},
        {"SETTINGS", true},
        {"EXIT TO MENU", true}
    };
}

Rectangle PauseMenuState::GetItemRect(int index, float sw, float sh) const {
    float itemW = sw * 0.3f, itemH = sh * 0.06f;
    float itemX = (sw - itemW) * 0.5f;
    return { itemX, sh * 0.4f + index * (itemH + sh * 0.03f), itemW, itemH };
}

void PauseMenuState::Update(float deltaTime) {
    timeAccum += deltaTime;
    float sw = (float)GetScreenWidth(), sh = (float)GetScreenHeight();
    
    // Keyboard Navigation
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        selectedIndex = (selectedIndex + 1) % entries.size();
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        selectedIndex = (selectedIndex - 1 + entries.size()) % entries.size();
    }
    
    // Mouse Interaction
    Vector2 mousePos = GetMousePosition();
    bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    
    if (mousePos.x != lastMousePos.x || mousePos.y != lastMousePos.y) {
        lastMousePos = mousePos;
        for (int i = 0; i < (int)entries.size(); i++) {
            if (CheckCollisionPointRec(mousePos, GetItemRect(i, sw, sh))) {
                selectedIndex = i;
            }
        }
    }
    
    if (mouseClicked) {
        for (int i = 0; i < (int)entries.size(); i++) {
            if (CheckCollisionPointRec(mousePos, GetItemRect(i, sw, sh))) {
                switch (i) {
                    case 0: // RESUME
                        Global::gameStateManager->PopState();
                        return;
                    case 1: // SAVE GAME
                        Global::gameStateManager->PushState(std::make_unique<SaveLoadState>(SaveLoadState::Mode::Save, parentState));
                        return;
                    case 2: // SETTINGS
                        Global::gameStateManager->PushState(std::make_unique<SettingsState>());
                        return;
                    case 3: // EXIT TO MENU
                        Global::gameStateManager->PopToMainMenu();
                        return;
                }
            }
        }
    }
    
    // Enter key action
    if (IsKeyPressed(Global::keys.select)) {
        switch (selectedIndex) {
            case 0: Global::gameStateManager->PopState(); return;
            case 1: Global::gameStateManager->PushState(std::make_unique<SaveLoadState>(SaveLoadState::Mode::Save, parentState)); return;
            case 2: Global::gameStateManager->PushState(std::make_unique<SettingsState>()); return;
            case 3: Global::gameStateManager->PopToMainMenu(); return;
        }
    }

    // Pressing ESC again resumes game
    if (IsKeyPressed(Global::keys.pause)) {
        Global::gameStateManager->PopState();
    }
}

void PauseMenuState::Draw() {
    // Draw the gameplay behind the pause menu
    if (parentState) {
        parentState->Draw();
    }
    
    // Dark overlay
    float sw = (float)GetScreenWidth(), sh = (float)GetScreenHeight();
    DrawRectangle(0, 0, (int)sw, (int)sh, Color{0, 0, 0, 150});
    
    // Draw Title
    int titleSize = (int)(sh * 0.1f);
    int titleW = MeasureText("PAUSED", titleSize);
    DrawText("PAUSED", (int)(sw - titleW) / 2, (int)(sh * 0.2f), titleSize, WHITE);
    
    // Draw Menu Items
    for (int i = 0; i < (int)entries.size(); i++) {
        Rectangle r = GetItemRect(i, sw, sh);
        Color textColor = (i == selectedIndex) ? YELLOW : WHITE;
        
        // Scale effect
        float scale = (i == selectedIndex) ? 1.0f + 0.05f * sinf(timeAccum * 5.0f) : 1.0f;
        int fontSize = (int)(r.height * 0.8f * scale);
        int textW = MeasureText(entries[i].label.c_str(), fontSize);
        
        DrawText(entries[i].label.c_str(), (int)(r.x + (r.width - textW) / 2), (int)(r.y + (r.height - fontSize) / 2), fontSize, textColor);
    }
}

void PauseMenuState::Cleanup() {}
