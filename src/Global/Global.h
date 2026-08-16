#pragma once
#include <memory>
#include "core/core.h"
#include "raylib.h"

namespace Global {
    constexpr float BASE_WIDTH = 800.0f;
    constexpr float BASE_HEIGHT = 600.0f;
    constexpr float GAME_SCALE = 3.0f;
    constexpr float TILE_SIZE = 16.0f;
    constexpr float MINI_PLAYER_WIDTH = 16.0f;
    constexpr float MINI_PLAYER_HEIGHT = 16.0f;
    constexpr float SUPER_PLAYER_WIDTH = 16.0f;
    constexpr float SUPER_PLAYER_HEIGHT = 32.0f;

    struct KeyBindings {
        int moveLeft  = KEY_LEFT;
        int moveRight = KEY_RIGHT;
        int jump      = KEY_SPACE;
        int action    = KEY_ENTER;
        int pause     = KEY_ESCAPE;
        int select    = KEY_ENTER;
        int reset     = KEY_R;
        int back      = KEY_ESCAPE;
    };

    extern std::unique_ptr<GameStateManager> gameStateManager;
    extern bool shouldExit;
    extern bool hasSaveGame;
    extern KeyBindings keys;
    extern Font titleFont;
    extern Font baseFont;

    void Init();
    void Cleanup();
}

// Override Raylib default font functions to use baseFont globally with outline
inline void DrawTextOutlined(Font font, const char* text, Vector2 pos, float fontSize, float spacing, Color color) {
    int outlineSize = 3;
    for (int dx = -outlineSize; dx <= outlineSize; dx++) {
        for (int dy = -outlineSize; dy <= outlineSize; dy++) {
            if (dx != 0 || dy != 0) {
                (DrawTextEx)(font, text, { pos.x + (float)dx, pos.y + (float)dy }, fontSize, spacing, BLACK);
            }
        }
    }
    (DrawTextEx)(font, text, pos, fontSize, spacing, color);
}

#define MeasureText(text, ...) ((int)MeasureTextEx(Global::baseFont, (text), (float)(__VA_ARGS__), 1.0f).x)
#define DrawTextEx(font, text, ...) DrawTextOutlined((font), (text), __VA_ARGS__)
#define DrawText(text, posX, posY, fontSize, ...) DrawTextOutlined(Global::baseFont, (text), { (float)(posX), (float)(posY) }, (float)(fontSize), 1.0f, (__VA_ARGS__))
