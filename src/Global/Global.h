#pragma once
#include <memory>
#include "core/core.h"

namespace Global {
    extern std::unique_ptr<GameStateManager> gameStateManager;
    extern bool shouldExit;

    void Init();
    void Cleanup();
}
