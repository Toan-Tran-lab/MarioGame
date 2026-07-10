#include "MainMenuState.h"
#include "MainMenu/SettingsState/SettingsState.h"
#include "Global/Global.h"

void MainMenuState::Initialize() {
    buttons.resize(BUTTON_COUNT);
    buttons[NEW_GAME] = Button({0, 0, 0, 0}, "New Game", DARKGREEN, GREEN);
    buttons[SETTINGS] = Button({0, 0, 0, 0}, "Settings", DARKBLUE, BLUE);
    buttons[EXIT]     = Button({0, 0, 0, 0}, "Exit",     MAROON, RED);
}

void MainMenuState::Update(float deltaTime) {
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();

    float btnW = sw * 0.25f;
    float btnH = sh * 0.07f;
    float btnX = (sw - btnW) * 0.5f;
    float startY = sh * 0.45f;
    float gap = sh * 0.03f;

    for (size_t i = 0; i < buttons.size(); i++) {
        float y = startY + i * (btnH + gap);
        buttons[i].SetBounds({ btnX, y, btnW, btnH });
    }

    Vector2 mouse = GetMousePosition();
    bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    for (auto& btn : buttons) btn.Update(mouse, mouseDown);

    if (buttons[NEW_GAME].IsClicked()) {
        Global::gameStateManager->PushState(std::make_unique<GameplayState>());
    }
    if (buttons[SETTINGS].IsClicked()) {
        Global::gameStateManager->PushState(std::make_unique<SettingsState>());
    }
    if (buttons[EXIT].IsClicked()) {
        Global::shouldExit = true;
    }
}

void MainMenuState::Draw() {
    ClearBackground(RAYWHITE);

    int fontSize = (int)(GetScreenHeight() * 0.08f);
    int textW = MeasureText("SUPER MARIO", fontSize);
    DrawText("SUPER MARIO", (GetScreenWidth() - textW) / 2,
             (int)(GetScreenHeight() * 0.15f), fontSize, RED);

    for (auto& btn : buttons) btn.Draw();
}

void MainMenuState::Cleanup() {
    buttons.clear();
}
