#include "Global.h"
#include "MainMenu/MainMenuState/MainMenuState.h"
#include "TextureManager/TextureManager.h"

namespace Global {
    std::unique_ptr<GameStateManager> gameStateManager;
    bool shouldExit = false;
    bool hasSaveGame = false;
    KeyBindings keys;

    void Init() {
        gameStateManager = std::make_unique<GameStateManager>();
        gameStateManager->PushState(std::make_unique<MainMenuState>());
    }

    void Cleanup() {
        gameStateManager.reset();
        TextureManager::UnloadAll();
    }
}
