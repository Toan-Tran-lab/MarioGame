#include "LevelSelectState.h"
#include "Global/Global.h"
#include "MainMenu/CharacterSelectState/CharacterSelectState.h"
#include "ui/UIUtils.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static bool LoadSandboxMapFromFile(const std::string& filePath, int& rows, int& cols, std::vector<std::vector<SandboxCellData>>& grid) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    try {
        json j;
        file >> j;
        rows = j.value("rows", 200);
        cols = j.value("cols", 200);
        
        grid.clear();
        grid.resize(rows, std::vector<SandboxCellData>(cols, {0, "", {0,0,0,0}, false}));

        for (const auto& jCell : j["grid"]) {
            int r = jCell["r"];
            int c = jCell["c"];
            if (r >= 0 && r < rows && c >= 0 && c < cols) {
                grid[r][c].type = jCell["type"];
                grid[r][c].texKey = jCell["texKey"];
                std::vector<float> rectVal = jCell["srcRect"];
                if (rectVal.size() == 4) {
                    grid[r][c].srcRect = { rectVal[0], rectVal[1], rectVal[2], rectVal[3] };
                }
                grid[r][c].isSolid = jCell.value("isSolid", true);
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}

void LevelSelectState::Initialize() {
    int total = Level::GetTotalLevels();
    levelButtons.resize(total);

    for (int i = 0; i < total; i++) {
        std::string levelText = "LEVEL " + std::to_string(i + 1);
        levelButtons[i] = Button({0, 0, 0, 0}, levelText, YELLOW, ORANGE);
    }

    hasCustomMap = FileExists("saves/sandbox_map.json");
    if (hasCustomMap) {
        customMapButton = Button({0, 0, 0, 0}, "CUSTOM MAP", Color{ 255, 160, 80, 255 }, ORANGE);
    }

    backButton = Button({0, 0, 0, 0}, "Back", MAROON, RED);
    selectedIndex = 0;
    timeAccum = 0.0f;
}

void LevelSelectState::Update(float deltaTime) {
    timeAccum += deltaTime;
    UIUtils::UpdateMenuBackground(deltaTime);
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();

    float btnW = sw * 0.30f;
    float btnH = sh * 0.08f;
    float btnX = (sw - btnW) * 0.5f;
    float startY = sh * 0.23f; // Slightly higher to fit all buttons neatly
    float gap = sh * 0.03f;

    // Layout level buttons
    for (size_t i = 0; i < levelButtons.size(); i++) {
        float y = startY + i * (btnH + gap);
        levelButtons[i].SetBounds({ btnX, y, btnW, btnH });
    }

    float nextY = startY + levelButtons.size() * (btnH + gap);

    // Layout custom map button
    if (hasCustomMap) {
        customMapButton.SetBounds({ btnX, nextY, btnW, btnH });
        nextY += (btnH + gap);
    }

    // Layout back button
    float backY = nextY + sh * 0.02f;
    backButton.SetBounds({ btnX, backY, btnW, btnH });

    int customIndex = (int)levelButtons.size();
    int backIndex = hasCustomMap ? (customIndex + 1) : customIndex;
    int totalItems = backIndex + 1;

    // Update all buttons (mouse hover)
    Vector2 mouse = GetMousePosition();
    bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    for (auto& btn : levelButtons) btn.Update(mouse, mouseDown);
    if (hasCustomMap) customMapButton.Update(mouse, mouseDown);
    backButton.Update(mouse, mouseDown);
    
    // Sync mouse hover to selectedIndex if mouse moved
    if (mouse.x != lastMousePos.x || mouse.y != lastMousePos.y) {
        lastMousePos = mouse;
        for (size_t i = 0; i < levelButtons.size(); i++) {
            if (CheckCollisionPointRec(mouse, levelButtons[i].GetBounds())) {
                selectedIndex = (int)i;
            }
        }
        if (hasCustomMap && CheckCollisionPointRec(mouse, customMapButton.GetBounds())) {
            selectedIndex = customIndex;
        }
        if (CheckCollisionPointRec(mouse, backButton.GetBounds())) {
            selectedIndex = backIndex;
        }
    }

    // Keyboard navigation
    if (IsKeyPressed(KEY_UP)) {
        selectedIndex = (selectedIndex - 1 + totalItems) % totalItems;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        selectedIndex = (selectedIndex + 1) % totalItems;
    }

    // Action Trigger helper
    auto confirmSelection = [&](int index) {
        if (index < (int)levelButtons.size()) {
            Level lvl = Level::GetLevel(index + 1);
            auto charSelect = std::make_unique<CharacterSelectState>();
            charSelect->SetLevel(lvl);
            Global::gameStateManager->PushState(std::move(charSelect));
        } else if (hasCustomMap && index == customIndex) {
            std::vector<std::vector<SandboxCellData>> cellGrid;
            int rCount = 200, cCount = 200;
            if (LoadSandboxMapFromFile("saves/sandbox_map.json", rCount, cCount, cellGrid)) {
                auto charSelect = std::make_unique<CharacterSelectState>();
                charSelect->SetSandboxMode(cellGrid);
                Global::gameStateManager->PushState(std::move(charSelect));
            }
        } else {
            // Back button selected
            Global::gameStateManager->PopState();
        }
    };

    // Confirm with Keyboard (ENTER/SPACE)
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        confirmSelection(selectedIndex);
        return;
    }

    // Confirm with Mouse clicks
    for (size_t i = 0; i < levelButtons.size(); i++) {
        if (levelButtons[i].IsClicked()) {
            confirmSelection((int)i);
            return;
        }
    }
    if (hasCustomMap && customMapButton.IsClicked()) {
        confirmSelection(customIndex);
        return;
    }

    // Back button or ESC → return to main menu
    if (backButton.IsClicked() || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
        confirmSelection(backIndex);
    }
}

void LevelSelectState::Draw() {
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();

    ClearBackground(Color{ 60, 40, 80, 255 });
    UIUtils::DrawMenuBackground(sw, sh);

    // Title with Drop Shadow
    int titleSize = (int)(sh * 0.06f);
    int titleY = (int)(sh * 0.08f); // Shifted slightly up
    
    // Draw drop shadow
    UIUtils::DrawCenteredTitle("SELECT LEVEL", titleY + 3, titleSize, Color{20, 10, 30, 255}, (int)sw);
    // Draw main title
    UIUtils::DrawCenteredTitle("SELECT LEVEL", titleY, titleSize, WHITE, (int)sw);
    
    // Draw a decorative line under the title (classic Mario style)
    float lineY = titleY + titleSize + 10;
    float lineW = sw * 0.4f;
    float lineX = (sw - lineW) * 0.5f;
    DrawRectangle((int)lineX, (int)lineY, (int)lineW, 4, GOLD);
    DrawRectangle((int)(lineX - 8), (int)(lineY - 2), 8, 8, RED);
    DrawRectangle((int)(lineX + lineW), (int)(lineY - 2), 8, 8, RED);

    // Draw buttons
    int menuFontSize = (int)(sh * 0.035f);
    for (size_t i = 0; i < levelButtons.size(); i++) {
        // Draw keyboard selection cursor
        if ((int)i == selectedIndex) {
            Rectangle r = levelButtons[i].GetBounds();
            int iconX = (int)(r.x - MeasureText(">", menuFontSize) - sw * 0.015f);
            int iconY = (int)(r.y + (r.height - menuFontSize) * 0.5f);
            UIUtils::DrawBlinkingText(">", iconX, iconY, menuFontSize, YELLOW, timeAccum);
        }
        levelButtons[i].Draw();
    }

    int customIndex = (int)levelButtons.size();
    int backIndex = hasCustomMap ? (customIndex + 1) : customIndex;

    // Draw custom map button
    if (hasCustomMap) {
        if (selectedIndex == customIndex) {
            Rectangle r = customMapButton.GetBounds();
            int iconX = (int)(r.x - MeasureText(">", menuFontSize) - sw * 0.015f);
            int iconY = (int)(r.y + (r.height - menuFontSize) * 0.5f);
            UIUtils::DrawBlinkingText(">", iconX, iconY, menuFontSize, YELLOW, timeAccum);
        }
        customMapButton.Draw();
    }

    // Draw back button with cursor if selected
    if (selectedIndex == backIndex) {
        Rectangle r = backButton.GetBounds();
        int iconX = (int)(r.x - MeasureText(">", menuFontSize) - sw * 0.015f);
        int iconY = (int)(r.y + (r.height - menuFontSize) * 0.5f);
        UIUtils::DrawBlinkingText(">", iconX, iconY, menuFontSize, YELLOW, timeAccum);
    }
    backButton.Draw();

    // Bottom bar with key hints
    int barY = (int)(sh * 0.92f);
    DrawLine(0, barY, (int)sw, barY, Color{ 100, 80, 120, 255 });
    int barFontSize = (int)(sh * 0.025f);
    int spacing = (int)(sw * 0.08f);
    float x = sw - spacing * 1.5f;

    UIUtils::DrawKeyPrompt("ESC", "BACK", x, barY + 5, barFontSize, spacing);
}

void LevelSelectState::Cleanup() {
    levelButtons.clear();
}
