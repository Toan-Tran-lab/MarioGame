#include "Global.h"
#include "MainMenu/MainMenuState/MainMenuState.h"
#include "TextureManager/TextureManager.h"
#include "SaveManager/SaveManager.h"

namespace Global {
    std::unique_ptr<GameStateManager> gameStateManager;
    bool shouldExit = false;
    bool hasSaveGame = false;
    KeyBindings keys;

    void Init() {
        gameStateManager = std::make_unique<GameStateManager>();
        gameStateManager->PushState(std::make_unique<MainMenuState>());

        hasSaveGame = SaveManager::SaveExists("auto_save") || 
                      SaveManager::SaveExists("slot1") || 
                      SaveManager::SaveExists("slot2") || 
                      SaveManager::SaveExists("slot3");
    }

    void Cleanup() {
        gameStateManager.reset();
        TextureManager::UnloadAll();
    }
}
