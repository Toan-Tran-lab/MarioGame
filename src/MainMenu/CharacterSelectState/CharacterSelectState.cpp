#include "CharacterSelectState.h"
#include "Global/Global.h"
#include <cmath>

void CharacterSelectState::Initialize() {
    timeAccum = 0.0f;
}

// Press ESC to return to the main menu
void CharacterSelectState::Update(float deltaTime) {
    timeAccum += deltaTime;

    if (IsKeyPressed(Global::keys.back) || IsKeyPressed(KEY_ESCAPE)) {
        Global::gameStateManager->PopState();
    }
}

// Draw the temporary character select screen (no content yet)
void CharacterSelectState::Draw() {
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();

    ClearBackground(Color{ 60, 40, 80, 255 });

    int titleSize = (int)(sh * 0.06f);
    const char* title = "SELECT CHARACTER";
    int titleW = MeasureText(title, titleSize);
    DrawText(title, (int)((sw - titleW) * 0.5f), (int)(sh * 0.4f), titleSize, WHITE);

    DrawText("Coming Soon", (int)((sw - MeasureText("Coming Soon", (int)(sh * 0.035f))) * 0.5f),
             (int)(sh * 0.5f), (int)(sh * 0.035f), Color{ 180, 180, 180, 255 });

    int barY = (int)(sh * 0.92f);
    DrawLine(0, barY, (int)sw, barY, Color{ 100, 80, 120, 255 });
    int barFontSize = (int)(sh * 0.025f);

    const char* selKey = "Enter";
    const char* selText = "SELECT";
    const char* backKey = "ESC";
    const char* backText = "BACK";

    int spacing = (int)(sw * 0.08f);
    float x = sw - spacing * 2;

    int selW = MeasureText(selKey, barFontSize);
    DrawText(selKey, (int)x, barY + 5, barFontSize, YELLOW);
    DrawText(selText, (int)(x + selW + 5), barY + 5, barFontSize, WHITE);

    x += spacing;
    int backW = MeasureText(backKey, barFontSize);
    DrawText(backKey, (int)x, barY + 5, barFontSize, YELLOW);
    DrawText(backText, (int)(x + backW + 5), barY + 5, barFontSize, WHITE);
}

void CharacterSelectState::Cleanup() {}
