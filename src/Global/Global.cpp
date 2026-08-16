#include "Global.h"
#include "MainMenu/MainMenuState/MainMenuState.h"
#include "TextureManager/TextureManager.h"
#include "SaveManager/SaveManager.h"

namespace Global {
    std::unique_ptr<GameStateManager> gameStateManager;
    bool shouldExit = false;
    bool hasSaveGame = false;
    KeyBindings keys;
    Font titleFont;
    Font baseFont;

    void Init() {
        titleFont = LoadFontEx("assets/fonts/SuperMario256.ttf", 128, 0, 250);
        baseFont = LoadFontEx("assets/fonts/PressStart2P-Regular.ttf", 64, 0, 250);
        gameStateManager = std::make_unique<GameStateManager>();
        gameStateManager->PushState(std::make_unique<MainMenuState>());

        hasSaveGame = SaveManager::SaveExists("auto_save") || 
                      SaveManager::SaveExists("slot1") || 
                      SaveManager::SaveExists("slot2") || 
                      SaveManager::SaveExists("slot3");
    }

    void Cleanup() {
        UnloadFont(titleFont);
        UnloadFont(baseFont);
        gameStateManager.reset();
        TextureManager::UnloadAll();
    }
}
