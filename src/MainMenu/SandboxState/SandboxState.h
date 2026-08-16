#pragma once
#include "core/core.h"
#include "Button/Button.h"
#include "Game Objects/Derived Objects/Playable Characters/Player/Mario.h"
#include "Game Objects/Derived Objects/Enemies/Goomba/Goomba.h"
#include "World/BlockGrid.h"
#include <vector>
#include <memory>

class SandboxState : public IGameState {
private:
    int cols_;
    int rows_;
    float cellSize_;
    std::vector<std::vector<SandboxCellData>> grid_;
    
    int selectedTool_; // 0 = Eraser, 1 = Dirt, 2 = Brick, 3 = Coin, 4 = Goomba
    float timeAccum_;

    struct PaletteItem {
        Rectangle rect;
        int type;
        const char* name;
        Color color;
    };
    std::vector<PaletteItem> palette_;
    Button playButton_;
    Button backButton_;
    Button saveMapButton_;
    Button loadMapButton_;

    // Playtest mode variables
    bool isPlaying_;
    std::unique_ptr<Mario> testPlayer_;
    BlockGrid tempBlockGrid_;

    // Camera variables
    Camera2D editorCam_;

    // Tileset Drawer variables
    bool drawerOpen_;
    int drawerActiveTab_; // 0 = BLOCK, 1 = DECOR, 2 = CHARACTER
    int drawerActiveSubTab_; // 0 = Level 1 (Overworld), 1 = Level 2 (Underground), 2 = Level 3 (Dungeon)
    float drawerScrollX_;
    float drawerScrollY_;
    float drawerZoom_;
    bool drawerDragging_;
    float drawerLastMouseX_;
    float drawerLastMouseY_;

    int lastSelectedTool_; // To restore brush tool when switching from Eraser

    struct SelectedTile {
        std::string texKey;
        Rectangle srcRect;
    };
    SelectedTile currentTile_;

    // Playtest entities
    std::vector<std::unique_ptr<Goomba>> playtestGoombas_;
    struct PlaytestCoin {
        Vector2 position;
        bool active;
    };
    std::vector<PlaytestCoin> playtestCoins_;

public:
    SandboxState();
    ~SandboxState() override = default;

    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Cleanup() override;

private:
    void DrawBlockPreview(int type, float x, float y, float alpha);
    void DrawBlock(int type, float x, float y, const std::string& texKey, Rectangle srcRect);
};
