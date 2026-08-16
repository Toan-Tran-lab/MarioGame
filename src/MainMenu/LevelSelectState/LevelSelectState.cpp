#include "LevelSelectState.h"
#include "Global/Global.h"
#include "MainMenu/CharacterSelectState/CharacterSelectState.h"
#include "ui/UIUtils.h"

void LevelSelectState::Initialize() {
    int total = Level::GetTotalLevels();
    levelButtons.resize(total);

    for (int i = 0; i < total; i++) {
        std::string levelText = "LEVEL " + std::to_string(i + 1);
        levelButtons[i] = Button({0, 0, 0, 0}, levelText, YELLOW, ORANGE);
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
    float startY = sh * 0.30f;
    float gap = sh * 0.04f;

    int totalItems = (int)levelButtons.size() + 1; // levels + back button

    // Layout level buttons
    for (size_t i = 0; i < levelButtons.size(); i++) {
        float y = startY + i * (btnH + gap);
        levelButtons[i].SetBounds({ btnX, y, btnW, btnH });
    }

    // Layout back button below the level buttons
    float backY = startY + levelButtons.size() * (btnH + gap) + sh * 0.03f;
    backButton.SetBounds({ btnX, backY, btnW, btnH });

    // Update all buttons (mouse hover)
    Vector2 mouse = GetMousePosition();
    bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    for (auto& btn : levelButtons) btn.Update(mouse, mouseDown);
    backButton.Update(mouse, mouseDown);
    
    // Sync mouse hover to selectedIndex if mouse moved
    if (mouse.x != lastMousePos.x || mouse.y != lastMousePos.y) {
        lastMousePos = mouse;
        for (size_t i = 0; i < levelButtons.size(); i++) {
            if (CheckCollisionPointRec(mouse, levelButtons[i].GetBounds())) {
                selectedIndex = (int)i;
            }
        }
        if (CheckCollisionPointRec(mouse, backButton.GetBounds())) {
            selectedIndex = (int)levelButtons.size();
        }
    }

    // Keyboard navigation
    if (IsKeyPressed(KEY_UP)) {
        selectedIndex = (selectedIndex - 1 + totalItems) % totalItems;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        selectedIndex = (selectedIndex + 1) % totalItems;
    }

    // ENTER/SPACE to confirm selection
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (selectedIndex < (int)levelButtons.size()) {
            Level lvl = Level::GetLevel(selectedIndex + 1);
            auto charSelect = std::make_unique<CharacterSelectState>();
            charSelect->SetLevel(lvl);
            Global::gameStateManager->PushState(std::move(charSelect));
            return;
        } else {
            // Back button selected
            Global::gameStateManager->PopState();
            return;
        }
    }

    // Check level button clicks (mouse)
    for (size_t i = 0; i < levelButtons.size(); i++) {
        if (levelButtons[i].IsClicked()) {
            Level lvl = Level::GetLevel((int)i + 1);
            auto charSelect = std::make_unique<CharacterSelectState>();
            charSelect->SetLevel(lvl);
            Global::gameStateManager->PushState(std::move(charSelect));
            return;
        }
    }

    // Back button or ESC → return to main menu
    if (backButton.IsClicked() || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
        Global::gameStateManager->PopState();
    }
}

void LevelSelectState::Draw() {
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();

    ClearBackground(Color{ 60, 40, 80, 255 });
    UIUtils::DrawMenuBackground(sw, sh);

    // Title with Drop Shadow
    int titleSize = (int)(sh * 0.06f);
    int titleY = (int)(sh * 0.12f);
    
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

    // Draw back button with cursor if selected
    if (selectedIndex == (int)levelButtons.size()) {
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
