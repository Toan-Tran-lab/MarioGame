#include "core.h"
#include "raylib.h"
#include "TextureManager/TextureManager.h"
#include "Global/Global.h"

void GameStateManager::PushState(std::unique_ptr<IGameState> state) {
    states.push(std::move(state));
    states.top()->Initialize();
}

void GameStateManager::PopState() {
    if (!states.empty()) {
        states.top()->Cleanup();
        states.pop();
    }
}

void GameStateManager::Update(float deltaTime) {
    if (!states.empty()) states.top()->Update(deltaTime);
}

void GameStateManager::Draw() {
    if (!states.empty()) states.top()->Draw();
}

// ======================================================
// GameplayState — test map + texture
// ======================================================

void GameplayState::SetLevel(const Level& level) {
    currentLevel = level;
}

void GameplayState::Initialize() {
    // Load map using the level's LDtk identifier
    tileMap.LoadFromLdtk("assets/maps/maps.ldtk", currentLevel.GetLdtkLevelId());

    // --- Khởi tạo player --
    int ts = tileMap.GetTileSize();
    playerWidth  = (float)ts;
    playerHeight = (float)ts;
    
    Vector2 spawn = tileMap.GetPlayerSpawn();
    if (spawn.x != 0 || spawn.y != 0) {
        playerX = spawn.x;
        playerY = spawn.y;
    } else {
        playerX = 3.0f * ts;                                    // Fallback
        playerY = (float)((tileMap.GetMapHeight() - 4) * ts);   // Fallback
    }
    playerSpeed = 300.0f;

    // --- Khởi tạo camera ---
    camera.Init((float)tileMap.GetPixelWidth(),
                (float)tileMap.GetPixelHeight());
}

void GameplayState::Update(float deltaTime) {
    int ts = tileMap.GetTileSize();

    // --- Player movement with tile collision ---
    float moveX = 0.0f, moveY = 0.0f;
    if (IsKeyDown(KEY_RIGHT)) moveX += 5 * playerSpeed * deltaTime;
    if (IsKeyDown(KEY_LEFT))  moveX -= 5 * playerSpeed * deltaTime;
    if (IsKeyDown(KEY_UP))    moveY -= 5 * playerSpeed * deltaTime;
    if (IsKeyDown(KEY_DOWN))  moveY += 5 * playerSpeed * deltaTime;

    // --- Resolve X axis first ---
    playerX += moveX;

    // Clamp to map boundaries
    if (playerX < 0) playerX = 0;
    if (playerX + playerWidth > tileMap.GetPixelWidth())
        playerX = tileMap.GetPixelWidth() - playerWidth;

    // Check collision on X axis
    {
        // Determine which tile columns/rows the player overlaps
        int colStart = (int)(playerX / ts);
        int colEnd   = (int)((playerX + playerWidth - 1) / ts);
        int rowStart = (int)(playerY / ts);
        int rowEnd   = (int)((playerY + playerHeight - 1) / ts);

        for (int row = rowStart; row <= rowEnd; row++) {
            for (int col = colStart; col <= colEnd; col++) {
                if (tileMap.IsSolidAt(col, row)) {
                    // Push player out of the solid tile
                    if (moveX > 0) {
                        // Moving right → push left edge of tile
                        playerX = (float)(col * ts) - playerWidth;
                    } else if (moveX < 0) {
                        // Moving left → push to right edge of tile
                        playerX = (float)((col + 1) * ts);
                    }
                }
            }
        }
    }

    // --- Resolve Y axis ---
    playerY += moveY;

    // Clamp to map boundaries
    if (playerY < 0) playerY = 0;
    if (playerY + playerHeight > tileMap.GetPixelHeight())
        playerY = tileMap.GetPixelHeight() - playerHeight;

    // Check collision on Y axis
    {
        int colStart = (int)(playerX / ts);
        int colEnd   = (int)((playerX + playerWidth - 1) / ts);
        int rowStart = (int)(playerY / ts);
        int rowEnd   = (int)((playerY + playerHeight - 1) / ts);

        for (int row = rowStart; row <= rowEnd; row++) {
            for (int col = colStart; col <= colEnd; col++) {
                if (tileMap.IsSolidAt(col, row)) {
                    if (moveY > 0) {
                        // Moving down → push to top edge of tile
                        playerY = (float)(row * ts) - playerHeight;
                    } else if (moveY < 0) {
                        // Moving up → push to bottom edge of tile
                        playerY = (float)((row + 1) * ts);
                    }
                }
            }
        }
    }

    // --- Cập nhật camera (theo dõi trung tâm player) ---
    camera.Update(playerX + playerWidth  / 2.0f,
                  playerY + playerHeight / 2.0f);

    // Chặn player đi lùi qua cạnh trái viewport (giống Mario thật)
    float leftEdge = camera.GetWorldLeft();
    if (playerX < leftEdge) playerX = leftEdge;

    // Nhấn BACKSPACE để quay lại menu
    if (IsKeyPressed(KEY_BACKSPACE)) {
        Global::gameStateManager->PopState();
    }
}

void GameplayState::Draw() {
    ClearBackground(tileMap.GetBackgroundColor());

    camera.BeginDraw();

        // Vẽ tilemap (truyền vị trí camera cho viewport culling)
        tileMap.Draw(camera.GetWorldLeft(), camera.GetWorldTop());

        // Vẽ player (tạm thời dùng hình chữ nhật đỏ)
        DrawRectangle((int)playerX, (int)playerY,
                      (int)playerWidth, (int)playerHeight, RED);

    camera.EndDraw();

    // HUD — vẽ ngoài camera để cố định trên màn hình
    DrawText(currentLevel.GetDisplayName().c_str(), 10, 10, 20, YELLOW);
    DrawText("Arrow keys: move player | Backspace: back to menu", 10, 35, 16, WHITE);
}

void GameplayState::Cleanup() {
    // Texture được quản lý bởi TextureManager, không cần giải phóng ở đây
}

