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

void GameplayState::Initialize() {
    // Load tileset texture
    TextureManager::Load("tiles",
        "assets/textures/Tileset/tileset2.png");

    // Khởi tạo bảng source rect cho các loại tile
    TileMap::InitTileInfoTable();

    // Load map từ file JSON
    tileMap.SetTextureKey("tiles");  // fallback nếu JSON không có textureKey
    tileMap.LoadFromJsonFile("assets/maps/level1.json");

    cameraX = 0.0f;
    cameraY = 0.0f;
}

void GameplayState::Update(float deltaTime) {
    // Di chuyển camera bằng phím mũi tên để test
    float speed = 200.0f;
    if (IsKeyDown(KEY_RIGHT)) cameraX += 5*speed * deltaTime;
    if (IsKeyDown(KEY_LEFT))  cameraX -= 5*speed * deltaTime;
    if (IsKeyDown(KEY_DOWN))  cameraY += 5*speed * deltaTime;
    if (IsKeyDown(KEY_UP))    cameraY -= 5*speed * deltaTime;

    // Giới hạn camera
    if (cameraX < 0) cameraX = 0;
    if (cameraY < 0) cameraY = 0;

    // Nhấn BACKSPACE để quay lại menu
    if (IsKeyPressed(KEY_BACKSPACE)) {
        Global::gameStateManager->PopState();
    }
}

void GameplayState::Draw() {
    ClearBackground(SKYBLUE);
    tileMap.Draw(cameraX, cameraY);

    // Hướng dẫn
    DrawText("Arrow keys: move camera | Backspace: back to menu", 10, 10, 16, WHITE);
}

void GameplayState::Cleanup() {
    // Texture được quản lý bởi TextureManager, không cần giải phóng ở đây
}

