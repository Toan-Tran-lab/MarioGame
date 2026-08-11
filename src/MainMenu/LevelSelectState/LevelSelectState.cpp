#include "LevelSelectState.h"
#include "Global/Global.h"

void LevelSelectState::Initialize() {
    int total = Level::GetTotalLevels();
    levelButtons.resize(total);

    // Color pairs for each level button
    Color normalColors[] = { DARKGREEN, DARKBLUE, DARKPURPLE };
    Color hoverColors[]  = { GREEN,     BLUE,     PURPLE };

    for (int i = 0; i < total; i++) {
        Level lvl = Level::GetLevel(i + 1);
        Color normal = normalColors[i % 3];
        Color hover  = hoverColors[i % 3];
        levelButtons[i] = Button({0, 0, 0, 0}, lvl.GetDisplayName(), normal, hover);
    }

    backButton = Button({0, 0, 0, 0}, "Back", MAROON, RED);
}

void LevelSelectState::Update(float deltaTime) {
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();

    float btnW = sw * 0.30f;
    float btnH = sh * 0.08f;
    float btnX = (sw - btnW) * 0.5f;
    float startY = sh * 0.30f;
    float gap = sh * 0.04f;

    // Layout level buttons
    for (size_t i = 0; i < levelButtons.size(); i++) {
        float y = startY + i * (btnH + gap);
        levelButtons[i].SetBounds({ btnX, y, btnW, btnH });
    }

    // Layout back button below the level buttons
    float backY = startY + levelButtons.size() * (btnH + gap) + sh * 0.03f;
    backButton.SetBounds({ btnX, backY, btnW, btnH });

    // Update all buttons
    Vector2 mouse = GetMousePosition();
    bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    for (auto& btn : levelButtons) btn.Update(mouse, mouseDown);
    backButton.Update(mouse, mouseDown);

    // Check level button clicks
    for (size_t i = 0; i < levelButtons.size(); i++) {
        if (levelButtons[i].IsClicked()) {
            Level lvl = Level::GetLevel((int)i + 1);
            auto gameplay = std::make_unique<GameplayState>();
            gameplay->SetLevel(lvl);
            Global::gameStateManager->PushState(std::move(gameplay));
            return;
        }
    }

    // Back button or BACKSPACE → return to main menu
    if (backButton.IsClicked() || IsKeyPressed(KEY_BACKSPACE)) {
        Global::gameStateManager->PopState();
    }
}

void LevelSelectState::Draw() {
    ClearBackground(RAYWHITE);

    // Title
    int fontSize = (int)(GetScreenHeight() * 0.07f);
    const char* title = "SELECT LEVEL";
    int textW = MeasureText(title, fontSize);
    DrawText(title, (GetScreenWidth() - textW) / 2,
             (int)(GetScreenHeight() * 0.12f), fontSize, DARKGRAY);

    // Draw buttons
    for (auto& btn : levelButtons) btn.Draw();
    backButton.Draw();

    // Hint
    DrawText("Backspace: back to menu", 10, 10, 16, GRAY);
}

void LevelSelectState::Cleanup() {
    levelButtons.clear();
}
