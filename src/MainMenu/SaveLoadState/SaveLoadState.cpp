#include "SaveLoadState.h"
#include "SaveManager/SaveManager.h"
#include "GameplayState/GameplayState.h"
#include "Global/Global.h"
#include "ui/UIUtils.h"

SaveLoadState::SaveLoadState(Mode m, GameplayState* gs) 
    : mode(m), gameplayState(gs), selectedIndex(0), timeAccum(0.0f) {}

void SaveLoadState::RefreshSlots() {
    slots.clear();
    std::string slotNames[] = {"auto_save", "slot1", "slot2", "slot3"};
    std::string displayNames[] = {"Auto Save", "Slot 1", "Slot 2", "Slot 3"};
    
    for (int i = 0; i < 4; i++) {
        // If mode is Save, we might want to prevent saving over auto_save directly,
        // but let's allow it or skip it. Let's skip auto_save for manual saving.
        if (mode == Mode::Save && i == 0) continue;
        
        SaveData data;
        bool exists = SaveManager::LoadGame(slotNames[i], data);
        std::string info = exists ? "Level " + std::to_string(data.levelId) + " - " + data.timestamp : "Empty";
        
        slots.push_back({displayNames[i], slotNames[i], info, exists});
    }
}

void SaveLoadState::Initialize() {
    RefreshSlots();
}

Rectangle SaveLoadState::GetItemRect(int index, float sw, float sh) const {
    float itemW = sw * 0.5f, itemH = sh * 0.1f;
    float itemX = (sw - itemW) * 0.5f;
    return { itemX, sh * 0.3f + index * (itemH + sh * 0.05f), itemW, itemH };
}

void SaveLoadState::Update(float deltaTime) {
    timeAccum += deltaTime;
    UIUtils::UpdateMenuBackground(deltaTime);
    float sw = (float)GetScreenWidth(), sh = (float)GetScreenHeight();
    
    if (IsKeyPressed(Global::keys.back)) {
        Global::gameStateManager->PopState();
        return;
    }
    
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        selectedIndex = (selectedIndex + 1) % slots.size();
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        selectedIndex = (selectedIndex - 1 + slots.size()) % slots.size();
    }
    
    Vector2 mousePos = GetMousePosition();
    bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    
    if (mousePos.x != lastMousePos.x || mousePos.y != lastMousePos.y) {
        lastMousePos = mousePos;
        for (int i = 0; i < (int)slots.size(); i++) {
            if (CheckCollisionPointRec(mousePos, GetItemRect(i, sw, sh))) {
                selectedIndex = i;
            }
        }
    }
    
    if (mouseClicked) {
        for (int i = 0; i < (int)slots.size(); i++) {
            if (CheckCollisionPointRec(mousePos, GetItemRect(i, sw, sh))) {
                // Trigger action
                if (mode == Mode::Save) {
                    if (gameplayState) {
                        SaveManager::SaveGame(slots[i].filename, gameplayState->GetSaveData());
                        RefreshSlots(); // Update UI
                        Global::hasSaveGame = true;
                    }
                } else { // Load
                    if (slots[i].exists) {
                        SaveData data;
                        if (SaveManager::LoadGame(slots[i].filename, data)) {
                            auto gs = std::make_unique<GameplayState>();
                            if (data.isSandboxMode) {
                                gs->SetSandboxMode(data.sandboxGrid);
                            } else {
                                gs->SetLevel(Level::GetLevel(data.levelId));
                            }
                            gs->SetLoadedData({data.playerX, data.playerY}, data.score, data.timeLeft);
                            Global::gameStateManager->PopToMainMenu(); // clear stack
                            Global::gameStateManager->PushState(std::move(gs));
                            return;
                        }
                    }
                }
            }
        }
    }
    
    if (IsKeyPressed(Global::keys.select)) {
        if (mode == Mode::Save) {
            if (gameplayState) {
                SaveManager::SaveGame(slots[selectedIndex].filename, gameplayState->GetSaveData());
                RefreshSlots();
                Global::hasSaveGame = true;
            }
        } else {
            if (slots[selectedIndex].exists) {
                SaveData data;
                if (SaveManager::LoadGame(slots[selectedIndex].filename, data)) {
                    auto gs = std::make_unique<GameplayState>();
                    if (data.isSandboxMode) {
                        gs->SetSandboxMode(data.sandboxGrid);
                    } else {
                        gs->SetLevel(Level::GetLevel(data.levelId));
                    }
                    gs->SetLoadedData({data.playerX, data.playerY}, data.score, data.timeLeft);
                    Global::gameStateManager->PopToMainMenu();
                    Global::gameStateManager->PushState(std::move(gs));
                    return;
                }
            }
        }
    }
}

void SaveLoadState::Draw() {
    // If in Save mode, we might want to draw the underlying game
    if (mode == Mode::Save && gameplayState) {
        gameplayState->Draw();
    } else {
        ClearBackground(DARKGRAY);
        UIUtils::DrawMenuBackground((float)GetScreenWidth(), (float)GetScreenHeight());
    }
    
    float sw = (float)GetScreenWidth(), sh = (float)GetScreenHeight();
    DrawRectangle(0, 0, (int)sw, (int)sh, Color{0, 0, 0, 150});
    
    int titleSize = (int)(sh * 0.08f);
    std::string title = (mode == Mode::Save) ? "SAVE GAME" : "LOAD GAME";
    UIUtils::DrawCenteredTitle(title.c_str(), (int)(sh * 0.1f), titleSize, WHITE, (int)sw);
    
    for (int i = 0; i < (int)slots.size(); i++) {
        Rectangle r = GetItemRect(i, sw, sh);
        Color bgColor = (i == selectedIndex) ? DARKBLUE : GRAY;
        if (mode == Mode::Load && !slots[i].exists) {
            bgColor = Color{50, 50, 50, 255};
        }
        
        DrawRectangleRounded(r, 0.1f, 10, bgColor);
        DrawRectangleRoundedLinesEx(r, 0.1f, 10, 2.0f, WHITE);
        
        int nameSize = (int)(r.height * 0.4f);
        DrawText(slots[i].name.c_str(), (int)(r.x + 20), (int)(r.y + 10), nameSize, WHITE);
        
        int infoSize = (int)(r.height * 0.3f);
        DrawText(slots[i].info.c_str(), (int)(r.x + 20), (int)(r.y + r.height - infoSize - 10), infoSize, LIGHTGRAY);
    }
    
    const char* hint = "Press ESC to back";
    DrawText(hint, (int)(sw - MeasureText(hint, 20)) / 2, (int)(sh * 0.9f), 20, LIGHTGRAY);
}

void SaveLoadState::Cleanup() {}
