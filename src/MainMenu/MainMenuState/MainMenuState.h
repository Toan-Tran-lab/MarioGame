#pragma once
#include "raylib.h"
#include "core/core.h"
#include "World/TileMap.h"
#include <vector>
#include <string>

// Main menu screen: New Game, Continue, Settings, and a quit confirmation dialog
class MainMenuState : public IGameState {
private:
    struct MenuEntry {
        std::string label;
        bool enabled;
    };

    std::vector<MenuEntry> entries;
    int selectedIndex = 0;
    float timeAccum = 0.0f;
    bool showExitPrompt = false;
    int exitChoice = 0;
    Vector2 lastMousePos = { -1.0f, -1.0f };

    // Background: rendered screenshot of Level 1 gameplay
    RenderTexture2D bgTexture;
    bool bgLoaded = false;

    Rectangle GetItemRect(int index, float sw, float sh) const;

    // Helper methods for Update (< 70 lines each)
    void UpdateExitPrompt(float sw, float sh, Vector2 mouse, bool clicked);
    void UpdateMenuSelection(float sw, float sh, Vector2 mouse);
    void HandleMenuAction();

    // Helper methods for Draw (< 70 lines each)
    void DrawBackground(float sw, float sh) const;
    void DrawMenuEntries(float sw, float sh) const;
    void DrawExitPromptDialog(float sw, float sh) const;

public:
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Cleanup() override;
};
