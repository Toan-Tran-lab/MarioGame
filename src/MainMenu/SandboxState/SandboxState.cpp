#include "SandboxState.h"
#include "Global/Global.h"
#include "TextureManager/TextureManager.h"
#include "ui/UIUtils.h"
#include "Game Objects/Derived Objects/Enemies/Goomba/Goomba.h"
#include "GameplayState/GameplayState.h"
#include <cmath>
#include <fstream>
#include <nlohmann/json.hpp>

#if defined(_WIN32)
#include <direct.h>
#define MKDIR(dir) _mkdir(dir)
#else
#include <sys/stat.h>
#define MKDIR(dir) mkdir(dir, 0777)
#endif

using json = nlohmann::json;

static void SaveSandboxMapToFile(const std::string& filePath, int rows, int cols, const std::vector<std::vector<SandboxCellData>>& grid) {
    MKDIR("saves");
    json jGrid = json::array();
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const auto& cell = grid[r][c];
            if (cell.type != 0) {
                json jCell;
                jCell["r"] = r;
                jCell["c"] = c;
                jCell["type"] = cell.type;
                jCell["texKey"] = cell.texKey;
                jCell["srcRect"] = { cell.srcRect.x, cell.srcRect.y, cell.srcRect.width, cell.srcRect.height };
                jCell["isSolid"] = cell.isSolid;
                jGrid.push_back(jCell);
            }
        }
    }
    json jOut;
    jOut["rows"] = rows;
    jOut["cols"] = cols;
    jOut["grid"] = jGrid;

    std::ofstream file(filePath);
    if (file.is_open()) {
        file << jOut.dump(4);
    }
}

static bool LoadSandboxMapFromFile(const std::string& filePath, int& rows, int& cols, std::vector<std::vector<SandboxCellData>>& grid) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    try {
        json j;
        file >> j;
        rows = j.value("rows", 200);
        cols = j.value("cols", 200);
        
        grid.clear();
        grid.resize(rows, std::vector<SandboxCellData>(cols, {0, "", {0,0,0,0}, false}));

        for (const auto& jCell : j["grid"]) {
            int r = jCell["r"];
            int c = jCell["c"];
            if (r >= 0 && r < rows && c >= 0 && c < cols) {
                grid[r][c].type = jCell["type"];
                grid[r][c].texKey = jCell["texKey"];
                std::vector<float> rectVal = jCell["srcRect"];
                if (rectVal.size() == 4) {
                    grid[r][c].srcRect = { rectVal[0], rectVal[1], rectVal[2], rectVal[3] };
                }
                grid[r][c].isSolid = jCell.value("isSolid", true);
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}

SandboxState::SandboxState()
    : cols_(200), rows_(200), cellSize_(48.0f), selectedTool_(1), timeAccum_(0.0f), isPlaying_(false),
      drawerOpen_(false), drawerActiveTab_(0), drawerActiveSubTab_(0), drawerScrollX_(0.0f), drawerScrollY_(0.0f), drawerZoom_(2.0f),
      drawerDragging_(false), drawerLastMouseX_(0.0f), drawerLastMouseY_(0.0f), lastSelectedTool_(1),
      currentTile_({ "ldtk_Terrain_1", { 16, 0, 16, 16 } }) {
}

void SandboxState::Initialize() {
    grid_.resize(rows_, std::vector<SandboxCellData>(cols_, {0, "", {0,0,0,0}, false}));
    
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();

    // Load tilesets by parsing maps.ldtk using temporary TileMaps (exactly like GameplayState does)
    TileMap tempL1, tempL2, tempL3;
    tempL1.LoadFromLdtk("assets/maps/maps.ldtk", "Level_0");
    tempL2.LoadFromLdtk("assets/maps/maps.ldtk", "Level_1");
    tempL3.LoadFromLdtk("assets/maps/maps.ldtk", "Level_2");
    
    // Load Goomba and Coin textures
    TextureManager::Load("goomba_texture", "assets/textures/enemies-3.png");
    TextureManager::Load("coin", "assets/textures/coin.png");

    // Setup Palette items: Eraser (type 0) and Selected Brush (type 1)
    palette_ = {
        { {340, 510, 48, 48}, 0, "Eraser", Color{150, 150, 150, 255} },
        { {412, 510, 48, 48}, 1, "Brush",  Color{120, 190, 80, 255} }
    };

    // Setup buttons
    backButton_ = Button({30, 514, 100, 40}, "BACK", MAROON, RED);
    saveMapButton_ = Button({145, 514, 85, 40}, "SAVE MAP", DARKBLUE, BLUE);
    loadMapButton_ = Button({245, 514, 85, 40}, "LOAD MAP", DARKPURPLE, PURPLE);
    playButton_ = Button({670, 514, 100, 40}, "PLAY", DARKGREEN, GREEN);

    // Load character textures if not loaded yet
    TextureManager::Load("mario_pose", "assets/textures/Mario/pose/mario.png");
    TextureManager::Load("mario_walk", "assets/textures/Mario/walk/mario.png");
    TextureManager::Load("mario_jump", "assets/textures/Mario/jump/mario.png");
    TextureManager::Load("mario_slide", "assets/textures/Mario/slide/mario.png");
    TextureManager::Load("mario_sit", "assets/textures/Mario/sit/mario.png");
    TextureManager::Load("mario_mini_pose", "assets/textures/Mario-mini/pose/mario.png");
    TextureManager::Load("mario_mini_walk", "assets/textures/Mario-mini/walk/mario.png");
    TextureManager::Load("mario_mini_jump", "assets/textures/Mario-mini/jump/mario.png");
    TextureManager::Load("mario_mini_slide", "assets/textures/Mario-mini/slide/mario.png");
    TextureManager::Load("mario_mini_die", "assets/textures/Mario-mini/die/mario.png");
    TextureManager::Load("mario_mini_sit", "assets/textures/Mario/sit/mario.png");

    // Initialize Editor Camera
    editorCam_.offset = { 0, 0 };
    editorCam_.target = { 0, (rows_ - 10) * cellSize_ }; // Start near the bottom floor
    editorCam_.rotation = 0.0f;
    editorCam_.zoom = 1.0f;
}

void SandboxState::Update(float deltaTime) {
    timeAccum_ += deltaTime;
    UIUtils::UpdateMenuBackground(deltaTime);

    Vector2 mouse = GetMousePosition();
    bool mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();
    float panelY = sh - 105.0f;

    // Dynamically position bottom panel buttons
    backButton_.SetBounds({ 30, panelY + 19, 100, 40 });
    saveMapButton_.SetBounds({ 145, panelY + 19, 85, 40 });
    loadMapButton_.SetBounds({ 245, panelY + 19, 85, 40 });
    playButton_.SetBounds({ sw - 130, panelY + 19, 100, 40 });

    // Dynamically position palette items
    if (palette_.size() >= 2) {
        palette_[0].rect = { 340, panelY + 15, 48, 48 };
        palette_[1].rect = { 412, panelY + 15, 48, 48 };
    }

    if (isPlaying_) {
        // --- Playtest Mode ---
        if (testPlayer_) {
            testPlayer_->Update(deltaTime);
            
            // Camera follows player during playtesting
            editorCam_.target = testPlayer_->GetPosition();
            editorCam_.offset = { sw / 2.0f, sh / 2.0f };

            // If player falls below the editor screen grid area, reset back above the floor
            if (testPlayer_->GetPosition().y > (rows_ + 2) * cellSize_) {
                // Find spawn cell, else default
                Vector2 spawnPos = { 400.0f, (rows_ - 3) * cellSize_ };
                for (int r = 0; r < rows_; ++r) {
                    for (int c = 0; c < cols_; ++c) {
                        if (grid_[r][c].type == 2) {
                            spawnPos = { 16.0f + c * cellSize_, 10.0f + r * cellSize_ };
                            break;
                        }
                    }
                }
                testPlayer_->SetPosition(spawnPos);
                testPlayer_->SyncPhysicsBody();
                
                // Clear and respawn playtest enemies/items on reset
                playtestGoombas_.clear();
                playtestCoins_.clear();
                for (int r = 0; r < rows_; ++r) {
                    for (int c = 0; c < cols_; ++c) {
                        if (grid_[r][c].type == 3) {
                            auto g = std::make_unique<Goomba>();
                            g->SetPosition({ 16.0f + c * cellSize_, 10.0f + r * cellSize_ });
                            g->SetSize({ Global::TILE_SIZE * Global::GAME_SCALE, Global::TILE_SIZE * Global::GAME_SCALE });
                            g->SetPlayerBody(&testPlayer_->GetPhysicsBody());
                            g->SetCollisionBlocks(&tempCollisions_);
                            playtestGoombas_.push_back(std::move(g));
                        } else if (grid_[r][c].type == 4) {
                            playtestCoins_.push_back({ { 16.0f + c * cellSize_, 10.0f + r * cellSize_ }, true });
                        }
                    }
                }
            }
        }

        // Update Goombas
        for (auto& g : playtestGoombas_) {
            if (g->IsActive()) {
                g->Update(deltaTime);
                if (!testPlayer_->IsDead() && testPlayer_->Overlaps(*g)) {
                    testPlayer_->InteractWith(*g);
                }
            }
        }

        // Update Coins
        for (auto& coin : playtestCoins_) {
            if (coin.active) {
                Rectangle pRect = testPlayer_->GetPhysicsBody().GetRect();
                Rectangle cRect = { coin.position.x, coin.position.y, cellSize_, cellSize_ };
                if (CheckCollisionRecs(pRect, cRect)) {
                    coin.active = false;
                }
            }
        }

        // Return to editor mode
        if (IsKeyPressed(KEY_ESCAPE)) {
            isPlaying_ = false;
            testPlayer_.reset();
            playtestGoombas_.clear();
            playtestCoins_.clear();
            // Reset camera back to standard editor layout
            editorCam_.offset = { 0, 0 };
            editorCam_.target = { 0, (rows_ - 10) * cellSize_ };
            editorCam_.zoom = 1.0f;
        }
        return;
    }

    // --- Left Drawer Update Logic ---
    // Handle drawer toggle button click (Y: 200 to 260, X depends on drawer state)
    Rectangle toggleRect = drawerOpen_ ? Rectangle{ 240, 200, 20, 60 } : Rectangle{ 0, 200, 20, 60 };
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, toggleRect)) {
        drawerOpen_ = !drawerOpen_;
        return; // Consume click
    }

    if (drawerOpen_) {
        // Consumes mouse clicks in drawer bounds to prevent placing blocks behind it
        if (mouse.x < 240 && mouse.y < 495) {
            // 1. Check main tab selection clicks (Y: 10 -> 40)
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouse.y >= 10 && mouse.y <= 40) {
                if (mouse.x < 80) drawerActiveTab_ = 0;      // BLOCK (SOLID)
                else if (mouse.x < 160) drawerActiveTab_ = 1; // DECOR (PASSTHROUGH)
                else if (mouse.x < 240) drawerActiveTab_ = 2; // CHARACTER (SPAWNERS)
                
                drawerScrollX_ = 0.0f;
                drawerScrollY_ = 0.0f; // Reset scrolls
            }

            // 2. Check sub-tab Level selection clicks for BLOCK & DECOR (Y: 50 -> 80)
            if ((drawerActiveTab_ == 0 || drawerActiveTab_ == 1) && 
                IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouse.y >= 50 && mouse.y <= 80) {
                if (mouse.x < 80) drawerActiveSubTab_ = 0;      // LV 1
                else if (mouse.x < 160) drawerActiveSubTab_ = 1; // LV 2
                else if (mouse.x < 240) drawerActiveSubTab_ = 2; // LV 3
                
                drawerScrollX_ = 0.0f;
                drawerScrollY_ = 0.0f; // Reset scrolls
            }

            // 3. Handle scrolling (Y) and zooming (with CTRL) inside the active area
            float viewStartY = (drawerActiveTab_ == 2) ? 50.0f : 90.0f;
            if (mouse.y >= viewStartY && mouse.y < 495) {
                float wheel = GetMouseWheelMove();
                if (wheel != 0) {
                    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
                        // Zooming (only applicable to Tileset sheets in BLOCK/DECOR)
                        if (drawerActiveTab_ == 0 || drawerActiveTab_ == 1) {
                            drawerZoom_ += wheel * 0.15f;
                            if (drawerZoom_ < 0.5f) drawerZoom_ = 0.5f;
                            if (drawerZoom_ > 5.0f) drawerZoom_ = 5.0f;
                        }
                    } else {
                        // Vertical Scrolling
                        float scrollSpeed = 40.0f;
                        drawerScrollY_ += wheel * scrollSpeed;
                    }
                }

                // Handle Middle Mouse Button Dragging (both X and Y directions)
                if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) {
                    if (!drawerDragging_) {
                        drawerDragging_ = true;
                        drawerLastMouseX_ = mouse.x;
                        drawerLastMouseY_ = mouse.y;
                    } else {
                        float dx = mouse.x - drawerLastMouseX_;
                        float dy = mouse.y - drawerLastMouseY_;
                        drawerScrollX_ += dx;
                        drawerScrollY_ += dy;
                        drawerLastMouseX_ = mouse.x;
                        drawerLastMouseY_ = mouse.y;
                    }
                } else {
                    drawerDragging_ = false;
                }
            }

            // 4. Selection Click Logic inside Drawer active tab
            if (drawerActiveTab_ == 0 || drawerActiveTab_ == 1) {
                // BLOCK or DECOR - Select tiles from sheets
                std::string activeTexKey = (drawerActiveSubTab_ == 0) ? "ldtk_Terrain_1" : 
                                           (drawerActiveSubTab_ == 1) ? "ldtk_Underground" : "ldtk_Dungeon";

                if (TextureManager::Has(activeTexKey)) {
                    Texture2D& tex = TextureManager::Get(activeTexKey);
                    float scale = drawerZoom_;
                    float drawnWidth = tex.width * scale;
                    float drawnHeight = tex.height * scale;
                    
                    float viewWidth = 200.0f; // from X: 20 to 220
                    float viewHeight = panelY - 90.0f; // viewport for sub-tabs
                    
                    float minScrollX = viewWidth - drawnWidth;
                    float minScrollY = viewHeight - drawnHeight;
                    if (minScrollX > 0) minScrollX = 0;
                    if (minScrollY > 0) minScrollY = 0;

                    // Clamp scroll values
                    if (drawerScrollX_ > 0) drawerScrollX_ = 0;
                    if (drawerScrollX_ < minScrollX) drawerScrollX_ = minScrollX;
                    if (drawerScrollY_ > 0) drawerScrollY_ = 0;
                    if (drawerScrollY_ < minScrollY) drawerScrollY_ = minScrollY;

                    // Click select tile (tileset drawn starting from X: 20 + drawerScrollX_)
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouse.y >= 90 && mouse.y < panelY && mouse.x >= 20 && mouse.x < 220) {
                        float localX = mouse.x - 20 - drawerScrollX_;
                        float localY = mouse.y - 90 - drawerScrollY_;

                        float texX = localX / scale;
                        float texY = localY / scale;

                        int tileX = (int)(texX / 17.0f);
                        int tileY = (int)(texY / 17.0f);

                        int maxTileX = (tex.width + 1) / 17;
                        int maxTileY = (tex.height + 1) / 17;
                        if (tileX >= 0 && tileX < maxTileX && tileY >= 0 && tileY < maxTileY) {
                            currentTile_.texKey = activeTexKey;
                            currentTile_.srcRect = { (float)tileX * 17.0f, (float)tileY * 17.0f, 16.0f, 16.0f };
                            selectedTool_ = 1; // Switch tool to Tile Brush
                            lastSelectedTool_ = 1;
                        }
                    }
                }
            } else if (drawerActiveTab_ == 2) {
                // CHARACTER - Select spawn entities (Mario, Goomba, Coin)
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouse.y < panelY && mouse.x >= 20 && mouse.x <= 220) {
                    if (mouse.y >= 70 && mouse.y <= 120) {
                        selectedTool_ = 2; // Mario Player Spawn
                        lastSelectedTool_ = 2;
                    } else if (mouse.y >= 140 && mouse.y <= 190) {
                        selectedTool_ = 3; // Goomba Spawner
                        lastSelectedTool_ = 3;
                    } else if (mouse.y >= 210 && mouse.y <= 260) {
                        selectedTool_ = 4; // Coin Spawner
                        lastSelectedTool_ = 4;
                    }
                }
            }
            return; // Block editing/buttons when using open drawer
        }
    }

    // --- Editor Mode ---
    // Update menu buttons
    backButton_.Update(mouse, mouseDown);
    saveMapButton_.Update(mouse, mouseDown);
    loadMapButton_.Update(mouse, mouseDown);
    playButton_.Update(mouse, mouseDown);

    if (backButton_.IsClicked() || IsKeyPressed(KEY_ESCAPE)) {
        Global::gameStateManager->PopState();
        return;
    }

    if (saveMapButton_.IsClicked()) {
        SaveSandboxMapToFile("saves/sandbox_map.json", rows_, cols_, grid_);
    }

    if (loadMapButton_.IsClicked()) {
        LoadSandboxMapFromFile("saves/sandbox_map.json", rows_, cols_, grid_);
    }

    // Camera Panning with Arrow Keys
    float camSpeed = 500.0f;
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) camSpeed *= 2.5f;

    if (IsKeyDown(KEY_LEFT)) editorCam_.target.x -= camSpeed * deltaTime;
    if (IsKeyDown(KEY_RIGHT)) editorCam_.target.x += camSpeed * deltaTime;
    if (IsKeyDown(KEY_UP)) editorCam_.target.y -= camSpeed * deltaTime;
    if (IsKeyDown(KEY_DOWN)) editorCam_.target.y += camSpeed * deltaTime;

    // Camera Zooming with Mouse Wheel
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        editorCam_.zoom += wheel * 0.08f;
        if (editorCam_.zoom < 0.2f) editorCam_.zoom = 0.2f;
        if (editorCam_.zoom > 3.0f) editorCam_.zoom = 3.0f;
    }

    // Clamping camera target within grid bounds
    float maxGridW = cols_ * cellSize_;
    float maxGridH = rows_ * cellSize_;
    if (editorCam_.target.x < 0) editorCam_.target.x = 0;
    if (editorCam_.target.x > maxGridW) editorCam_.target.x = maxGridW;
    if (editorCam_.target.y < 0) editorCam_.target.y = 0;
    if (editorCam_.target.y > maxGridH) editorCam_.target.y = maxGridH;

    // Round target coordinates to integer to prevent sub-pixel blur during camera movement
    editorCam_.target.x = roundf(editorCam_.target.x);
    editorCam_.target.y = roundf(editorCam_.target.y);

    if (playButton_.IsClicked()) {
        // Create SandboxCellData grid representation to transfer to GameplayState
        std::vector<std::vector<SandboxCellData>> cellGrid(rows_, std::vector<SandboxCellData>(cols_));
        for (int r = 0; r < rows_; ++r) {
            for (int c = 0; c < cols_; ++c) {
                cellGrid[r][c].type = grid_[r][c].type;
                cellGrid[r][c].texKey = grid_[r][c].texKey;
                cellGrid[r][c].srcRect = grid_[r][c].srcRect;
                cellGrid[r][c].isSolid = grid_[r][c].isSolid;
            }
        }

        auto playState = std::make_unique<GameplayState>();
        // Set character selected in menu if available (or default to Mario)
        playState->SetCharacter(0); // Default to Mario (0 = Mario, 1 = Luigi)
        playState->SetSandboxMode(cellGrid);

        // Push state so that pausing, quitting, or dying gracefully pops back to Sandbox editor!
        Global::gameStateManager->PushState(std::move(playState));
        return;
    }

    // Check click on palette items (Eraser or Selected Brush tool)
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        for (const auto& item : palette_) {
            if (CheckCollisionPointRec(mouse, item.rect)) {
                if (item.type == 0) {
                    selectedTool_ = 0; // Select Eraser
                } else {
                    selectedTool_ = lastSelectedTool_; // Restore selected brush tool (1, 2, 3, or 4)
                }
                return;
            }
        }
    }

    // Edit grid blocks (Only when not clicking on UI area Y > 495 and X > left bounds)
    float gridLeftBound = drawerOpen_ ? 240.0f : 20.0f;
    if (mouse.y < panelY && mouse.x > gridLeftBound) {
        Vector2 worldMouse = GetScreenToWorld2D(mouse, editorCam_);
        int col = (int)((worldMouse.x - 16.0f) / cellSize_);
        int row = (int)((worldMouse.y - 10.0f) / cellSize_);

        if (col >= 0 && col < cols_ && row >= 0 && row < rows_) {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                if (selectedTool_ == 2) {
                    // Only one Player Spawn allowed! Clear other Player Spawn points
                    for (int r = 0; r < rows_; ++r) {
                        for (int c = 0; c < cols_; ++c) {
                            if (grid_[r][c].type == 2) {
                                grid_[r][c].type = 0;
                            }
                        }
                    }
                }
                grid_[row][col].type = selectedTool_;
                if (selectedTool_ == 1) { // Tile brush
                    grid_[row][col].texKey = currentTile_.texKey;
                    grid_[row][col].srcRect = currentTile_.srcRect;
                    grid_[row][col].isSolid = (drawerActiveTab_ == 0); // Solid in tab 0 (BLOCK), Decor in tab 1 (DECOR)
                }
            } else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
                grid_[row][col].type = 0; // Erase
            }
        }
    }
}

void SandboxState::Draw() {
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();
    Vector2 mouse = GetMousePosition();

    ClearBackground(Color{ 40, 20, 60, 255 });
    UIUtils::DrawMenuBackground(sw, sh);

    // Calculate frustum culling boundaries (only render visible blocks to prevent lag)
    float panelY = sh - 105.0f;
    Vector2 tl = GetScreenToWorld2D({0, 0}, editorCam_);
    Vector2 br = GetScreenToWorld2D({sw, panelY}, editorCam_); // Use panelY instead of hardcoded 495

    int startCol = (int)((tl.x - 16.0f) / cellSize_) - 1;
    int endCol = (int)((br.x - 16.0f) / cellSize_) + 1;
    int startRow = (int)((tl.y - 10.0f) / cellSize_) - 1;
    int endRow = (int)((br.y - 10.0f) / cellSize_) + 1;

    if (startCol < 0) startCol = 0;
    if (endCol >= cols_) endCol = cols_ - 1;
    if (startRow < 0) startRow = 0;
    if (endRow >= rows_) endRow = rows_ - 1;

    // --- Enter Camera Space ---
    BeginMode2D(editorCam_);

    // 1. Draw grid background (dark gray for better contrast)
    DrawRectangle(16, 10, cols_ * cellSize_, rows_ * cellSize_, Color{ 50, 50, 50, 255 });
    
    // 2. Draw placed blocks and entities (culling applied)
    for (int r = startRow; r <= endRow; ++r) {
        for (int c = startCol; c <= endCol; ++c) {
            int t = grid_[r][c].type;
            if (t != 0) {
                // If it is a pass-through decoration tile, draw it slightly translucent so the player knows
                if (t == 1 && !grid_[r][c].isSolid) {
                    // Draw decoration tile
                    DrawBlock(t, 16.0f + c * cellSize_, 10.0f + r * cellSize_, grid_[r][c].texKey, grid_[r][c].srcRect);
                    // Draw a subtle 'D' or overlay just to distinguish it during editor mode
                    if (!isPlaying_) {
                        DrawRectangleLinesEx({ 16.0f + c * cellSize_, 10.0f + r * cellSize_, cellSize_, cellSize_ }, 1.0f, Color{ 80, 160, 240, 100 });
                    }
                } else {
                    DrawBlock(t, 16.0f + c * cellSize_, 10.0f + r * cellSize_, grid_[r][c].texKey, grid_[r][c].srcRect);
                }
            }
        }
    }

    // 3. Draw outline frame for the entire 500x500 grid
    DrawRectangleLinesEx({16, 10, cols_ * cellSize_, rows_ * cellSize_}, 4.0f, Color{100, 80, 140, 255});

    if (isPlaying_) {
        // Draw test player under camera space
        if (testPlayer_) {
            testPlayer_->Draw();
        }
        // Draw active Goombas
        for (auto& g : playtestGoombas_) {
            if (g->IsActive()) {
                g->Draw();
            }
        }
        // Draw active Coins
        for (auto& coin : playtestCoins_) {
            if (coin.active) {
                DrawBlockPreview(4, coin.position.x, coin.position.y, 1.0f);
            }
        }
    } else {
        // Draw preview block & grid outline ONLY on the hovered cell
        float gridLeftBound = drawerOpen_ ? 240.0f : 20.0f;
        if (mouse.y < panelY && mouse.x > gridLeftBound) {
            Vector2 worldMouse = GetScreenToWorld2D(mouse, editorCam_);
            int hCol = (int)((worldMouse.x - 16.0f) / cellSize_);
            int hRow = (int)((worldMouse.y - 10.0f) / cellSize_);

            if (hCol >= 0 && hCol < cols_ && hRow >= 0 && hRow < rows_) {
                float bx = 16.0f + hCol * cellSize_;
                float by = 10.0f + hRow * cellSize_;
                
                // Draw semi-transparent preview of selected tool
                DrawBlockPreview(selectedTool_, bx, by, 0.5f);
                // Draw grid lines ONLY on this specific cell (prevents screen clutter)
                DrawRectangleLinesEx({bx, by, cellSize_, cellSize_}, 1.5f, Color{160, 160, 160, 180});
            }
        }
    }

    EndMode2D();
    // --- Exit Camera Space ---

    // --- Draw Static UI (Always overlayed, not affected by camera) ---
    if (isPlaying_) {
        // Draw HUD message during playtesting
        DrawRectangle(0, 0, (int)sw, 40, Color{0, 0, 0, 160});
        DrawText("PLAYTESTING MODE - Press ESC to return to Editor", 20, 10, 20, GREEN);
        return;
    }

    // Draw bottom control panel background (opaque to block tiles underneath)
    DrawRectangle(0, (int)panelY, (int)sw, 105, Color{ 25, 15, 35, 255 });
    DrawLine(0, (int)panelY, (int)sw, (int)panelY, Color{ 120, 90, 160, 255 });

    // Draw Back, Save, Load and Play buttons
    backButton_.Draw();
    saveMapButton_.Draw();
    loadMapButton_.Draw();
    playButton_.Draw();

    // Draw palette selector items (Eraser and Active Brush)
    for (const auto& item : palette_) {
        // Background rectangle
        DrawRectangleRec(item.rect, Color{35, 25, 45, 255});
        DrawRectangleLinesEx(item.rect, 1.0f, Color{80, 60, 100, 255});

        float bx = item.rect.x + 4;
        float by = item.rect.y + 4;
        float bSize = 40.0f;

        if (item.type == 0) {
            // Eraser representation
            DrawRectangleRec({bx, by, bSize, bSize}, Color{80, 80, 80, 255});
            DrawLine(bx, by, bx + bSize, by + bSize, RED);
            DrawLine(bx + bSize, by, bx, by + bSize, RED);
        } else if (item.type == 1) {
            // Draw current active brush tool representation
            float oldCellSize = cellSize_;
            cellSize_ = 40.0f;
            DrawBlockPreview(lastSelectedTool_, bx, by, 1.0f);
            cellSize_ = oldCellSize;
        }

        // Highlight if active
        bool isThisSelected = (item.type == 0 && selectedTool_ == 0) || (item.type == 1 && selectedTool_ != 0);
        if (isThisSelected) {
            DrawRectangleLinesEx(item.rect, 3.0f, GOLD);
        }
    }

    // HUD status text
    DrawText("Arrows: Move Cam  |  Scroll: Zoom  |  L-Click: Draw  |  R-Click: Erase", 180, (int)(panelY + 73), 16, LIGHTGRAY);

    // --- Draw Left Drawer ---
    if (drawerOpen_) {
        // Draw Drawer Panel background
        DrawRectangle(0, 0, 240, (int)panelY, Color{ 30, 25, 40, 255 });
        DrawLine(240, 0, 240, (int)panelY, Color{ 120, 90, 160, 255 });
        
        // Draw main categories tabs
        for (int t = 0; t < 3; ++t) {
            Rectangle tabRect = { (float)t * 80.0f, 10, 80, 30 };
            Color tabCol = (t == drawerActiveTab_) ? Color{ 80, 60, 110, 255 } : Color{ 45, 35, 55, 255 };
            DrawRectangleRec(tabRect, tabCol);
            DrawRectangleLinesEx(tabRect, 1.0f, Color{ 120, 90, 160, 255 });
            const char* tabLabel = (t == 0) ? "BLOCK" : (t == 1) ? "DECOR" : "CHAR";
            DrawText(tabLabel, tabRect.x + 13, tabRect.y + 7, 15, WHITE);
        }

        if (drawerActiveTab_ == 0 || drawerActiveTab_ == 1) {
            // Draw Sub-tabs for Level selector (LV 1, LV 2, LV 3)
            for (int s = 0; s < 3; ++s) {
                Rectangle subTabRect = { (float)s * 80.0f, 50, 80, 30 };
                Color subTabCol = (s == drawerActiveSubTab_) ? Color{ 60, 100, 120, 255 } : Color{ 35, 45, 55, 255 };
                DrawRectangleRec(subTabRect, subTabCol);
                DrawRectangleLinesEx(subTabRect, 1.0f, Color{ 90, 120, 160, 255 });
                const char* subTabLabel = (s == 0) ? "LV 1" : (s == 1) ? "LV 2" : "LV 3";
                DrawText(subTabLabel, subTabRect.x + 22, subTabRect.y + 7, 15, WHITE);
            }

            // Draw active tileset sheet with scroll scissor
            BeginScissorMode(20, 90, 200, (int)(panelY - 90.0f));
            std::string activeTexKey = (drawerActiveSubTab_ == 0) ? "ldtk_Terrain_1" : 
                                       (drawerActiveSubTab_ == 1) ? "ldtk_Underground" : "ldtk_Dungeon";
            
            if (TextureManager::Has(activeTexKey)) {
                Texture2D& tex = TextureManager::Get(activeTexKey);
                float scale = drawerZoom_;
                float drawnWidth = tex.width * scale;
                float drawnHeight = tex.height * scale;
                
                DrawTextureEx(tex, { 20 + drawerScrollX_, 90 + drawerScrollY_ }, 0.0f, scale, WHITE);

                // Draw grid lines on the tileset taking spacing into account
                int colsCount = (tex.width + 1) / 17;
                int rowsCount = (tex.height + 1) / 17;
                for (int r = 0; r < rowsCount; ++r) {
                    for (int c = 0; c < colsCount; ++c) {
                        float tx = c * 17.0f;
                        float ty = r * 17.0f;
                        DrawRectangleLinesEx({
                            20 + drawerScrollX_ + tx * scale,
                            90 + drawerScrollY_ + ty * scale,
                            16.0f * scale,
                            16.0f * scale
                        }, 0.8f, Color{ 255, 255, 255, 40 });
                    }
                }

                // Highlight selected tile
                if (currentTile_.texKey == activeTexKey) {
                    Rectangle selRect = {
                        20 + drawerScrollX_ + currentTile_.srcRect.x * scale,
                        90 + drawerScrollY_ + currentTile_.srcRect.y * scale,
                        16 * scale,
                        16 * scale
                    };
                    DrawRectangleLinesEx(selRect, 2.0f, GOLD);
                }
            }
            EndScissorMode();
            
            DrawText("Ctrl+Wheel: Zoom  |  Mid-Btn: Pan", 10, (int)(panelY - 17), 12, LIGHTGRAY);
        } else if (drawerActiveTab_ == 2) {
            // CHARACTER - List spawn entities
            // 1. Player Spawner (represented as a light orange square block)
            Rectangle mRect = { 20, 70, 200, 50 };
            DrawRectangleRec(mRect, (lastSelectedTool_ == 2) ? Color{ 80, 50, 50, 255 } : Color{ 40, 35, 45, 255 });
            DrawRectangleLinesEx(mRect, 1.5f, (lastSelectedTool_ == 2) ? GOLD : Color{ 80, 60, 100, 255 });
            DrawRectangleRec({ 30, 80, 30, 30 }, Color{ 255, 160, 80, 255 });
            DrawRectangleLinesEx({ 30, 80, 30, 30 }, 1.0f, GOLD);
            DrawText("P", 41, 88, 14, WHITE);
            DrawText("Player Start", 75, 85, 16, WHITE);

            // 2. Goomba spawner
            Rectangle gRect = { 20, 140, 200, 50 };
            DrawRectangleRec(gRect, (lastSelectedTool_ == 3) ? Color{ 80, 50, 50, 255 } : Color{ 40, 35, 45, 255 });
            DrawRectangleLinesEx(gRect, 1.5f, (lastSelectedTool_ == 3) ? GOLD : Color{ 80, 60, 100, 255 });
            if (TextureManager::Has("goomba_texture")) {
                Texture2D& tex = TextureManager::Get("goomba_texture");
                DrawTexturePro(tex, { 0, 0, 16, 16 }, { 30, 150, 30, 30 }, { 0, 0 }, 0.0f, WHITE);
            }
            DrawText("Goomba Spawn", 75, 155, 16, WHITE);

            // 3. Coin spawner
            Rectangle cRect = { 20, 210, 200, 50 };
            DrawRectangleRec(cRect, (lastSelectedTool_ == 4) ? Color{ 80, 50, 50, 255 } : Color{ 40, 35, 45, 255 });
            DrawRectangleLinesEx(cRect, 1.5f, (lastSelectedTool_ == 4) ? GOLD : Color{ 80, 60, 100, 255 });
            if (TextureManager::Has("coin")) {
                Texture2D& tex = TextureManager::Get("coin");
                DrawTexturePro(tex, { 0, 0, (float)tex.width, (float)tex.height }, { 30, 220, 30, 30 }, { 0, 0 }, 0.0f, WHITE);
            }
            DrawText("Coin Collect", 75, 225, 16, WHITE);
        }
    }

    // Draw drawer arrow toggle button
    Rectangle toggleRect = drawerOpen_ ? Rectangle{ 240, panelY / 2.0f - 30.0f, 20, 60 } : Rectangle{ 0, panelY / 2.0f - 30.0f, 20, 60 };
    DrawRectangleRec(toggleRect, Color{ 55, 45, 75, 255 });
    DrawRectangleLinesEx(toggleRect, 1.5f, Color{ 120, 90, 160, 255 });
    const char* arrowText = drawerOpen_ ? "<" : ">";
    DrawText(arrowText, toggleRect.x + 6, toggleRect.y + 20, 20, WHITE);
}

void SandboxState::DrawBlock(int type, float x, float y, const std::string& texKey, Rectangle srcRect) {
    if (type == 1) { // Tile Brush block
        if (TextureManager::Has(texKey)) {
            Texture2D& tex = TextureManager::Get(texKey);
            Rectangle dest = { x, y, cellSize_, cellSize_ };
            DrawTexturePro(tex, srcRect, dest, {0,0}, 0.0f, WHITE);
        }
    } else {
        DrawBlockPreview(type, x, y, 1.0f);
    }
}

void SandboxState::DrawBlockPreview(int type, float x, float y, float alpha) {
    if (type == 0) {
        // Eraser preview
        DrawRectangleRoundedLinesEx({x, y, cellSize_, cellSize_}, 0.15f, 10, 2.0f, Fade(RED, alpha));
        DrawLine(x, y, x + cellSize_, y + cellSize_, Fade(RED, alpha));
        DrawLine(x + cellSize_, y, x, y + cellSize_, Fade(RED, alpha));
        return;
    }

    if (type == 1) {
        // Tile Brush (draws current selected tile from drawer)
        if (TextureManager::Has(currentTile_.texKey)) {
            Texture2D& tex = TextureManager::Get(currentTile_.texKey);
            Rectangle dest = { x, y, cellSize_, cellSize_ };
            DrawTexturePro(tex, currentTile_.srcRect, dest, {0,0}, 0.0f, Fade(WHITE, alpha));
        }
    }
    else if (type == 2) {
        // Player Spawn (light orange square with "PLAYER" written on it)
        DrawRectangleRec({ x, y, cellSize_, cellSize_ }, Fade(Color{ 255, 160, 80, 255 }, alpha));
        DrawRectangleLinesEx({ x, y, cellSize_, cellSize_ }, 1.5f, Fade(GOLD, alpha));
        
        int fontSize = (int)(cellSize_ * 0.22f);
        int textW = MeasureText("PLAYER", fontSize);
        DrawText("PLAYER", (int)(x + (cellSize_ - textW) / 2), (int)(y + (cellSize_ - fontSize) / 2), fontSize, Fade(WHITE, alpha));
    }
    else if (type == 3) {
        // Goomba spawner
        if (TextureManager::Has("goomba_texture")) {
            Texture2D& tex = TextureManager::Get("goomba_texture");
            Rectangle src = { 0, 0, 16, 16 };
            Rectangle dest = { x, y, cellSize_, cellSize_ };
            DrawTexturePro(tex, src, dest, {0,0}, 0.0f, Fade(WHITE, alpha));
        }
    }
    else if (type == 4) {
        // Coin spawner
        if (TextureManager::Has("coin")) {
            Texture2D& tex = TextureManager::Get("coin");
            Rectangle src = { 0, 0, (float)tex.width, (float)tex.height };
            Rectangle dest = { x + cellSize_*0.15f, y + cellSize_*0.15f, cellSize_*0.7f, cellSize_*0.7f };
            DrawTexturePro(tex, src, dest, {0,0}, 0.0f, Fade(WHITE, alpha));
        }
    }
}

void SandboxState::Cleanup() {
    grid_.clear();
    palette_.clear();
    testPlayer_.reset();
    playtestGoombas_.clear();
    playtestCoins_.clear();
}
