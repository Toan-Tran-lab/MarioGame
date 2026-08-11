#pragma once
#include <memory>
#include "core/core.h"
#include "raylib.h"

namespace Global {
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

    void Init();
    void Cleanup();
}
