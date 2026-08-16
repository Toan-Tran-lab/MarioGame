#include "GameplayState.h"
#include "raylib.h"
#include "TextureManager/TextureManager.h"
#include "Global/Global.h"
#include "physics/PhysicsEngine.h"
#include "physics/CollisionSystem.h"
#include "physics/ProximityAI.h"
#include "SaveManager/SaveManager.h"
#include "MainMenu/PauseMenuState/PauseMenuState.h"
#include "Game Objects/Derived Objects/Playable Characters/Player/Mario.h"
#include "Game Objects/Derived Objects/Playable Characters/Player/Luigi.h"
#include <iostream>

GameplayState::GameplayState() {
    currentLevel = Level::GetLevel(1);
    player_ = std::make_unique<Mario>();
}

void GameplayState::SetLevel(const Level& level) {
    currentLevel = level;
}

void GameplayState::SetCharacter(int characterId) {
    if (characterId == 1) {
        player_ = std::make_unique<Luigi>();
    } else {
        player_ = std::make_unique<Mario>();
    }
}

void GameplayState::SetLoadedData(Vector2 pos, int loadedScore, float loadedTime) {
    player_->SetPosition(pos);
    score = loadedScore;
    timeLeft = loadedTime;
    // We can use a flag or just rely on position.
    // If the position is not (0,0), we might skip spawn overriding in Initialize.
}

SaveData GameplayState::GetSaveData() const {
    SaveData data;
    data.levelId = currentLevel.GetLevelNumber();
    data.playerX = player_->GetPosition().x;
    data.playerY = player_->GetPosition().y;
    data.score = score;
    data.timeLeft = timeLeft;
    return data;
}

void GameplayState::Initialize() {
    // Load the tilemap based on current level
    std::string ldtkPath = "assets/maps/maps.ldtk";
    if (!tileMap.LoadFromLdtk(ldtkPath, currentLevel.GetLdtkLevelId())) {
        std::cerr << "Failed to load tilemap for level: " << currentLevel.GetLdtkLevelId() << std::endl;
    }

    // Load player animations/textures
    // Mario Normal
    TextureManager::Load("mario_pose", "assets/textures/Mario/pose/mario.png");
    TextureManager::Load("mario_walk", "assets/textures/Mario/walk/mario.png");
    TextureManager::Load("mario_jump", "assets/textures/Mario/jump/mario.png");
    TextureManager::Load("mario_slide", "assets/textures/Mario/slide/mario.png");
    TextureManager::Load("mario_sit", "assets/textures/Mario/sit/mario.png");

    // Mario Mini
    TextureManager::Load("mario_mini_pose", "assets/textures/Mario-mini/pose/mario.png");
    TextureManager::Load("mario_mini_walk", "assets/textures/Mario-mini/walk/mario.png");
    TextureManager::Load("mario_mini_jump", "assets/textures/Mario-mini/jump/mario.png");
    TextureManager::Load("mario_mini_slide", "assets/textures/Mario-mini/slide/mario.png");
    TextureManager::Load("mario_mini_die", "assets/textures/Mario-mini/die/mario.png");
    TextureManager::Load("mario_mini_sit", "assets/textures/Mario/sit/mario.png"); // using normal sit if mini sit isn't provided

    // Luigi Normal
    TextureManager::Load("luigi_pose", "assets/textures/Luigi/pose/luigi.png");
    TextureManager::Load("luigi_walk", "assets/textures/Luigi/walk/luigi.png");
    TextureManager::Load("luigi_jump", "assets/textures/Luigi/jump/luigi.png");
    TextureManager::Load("luigi_slide", "assets/textures/Luigi/slide/luigi.png");
    TextureManager::Load("luigi_sit", "assets/textures/Luigi/sit/luigi.png");

    // Luigi Mini
    TextureManager::Load("luigi_mini_pose", "assets/textures/Luigi-mini/pose/luigi.png");
    TextureManager::Load("luigi_mini_walk", "assets/textures/Luigi-mini/walk/luigi.png");
    TextureManager::Load("luigi_mini_jump", "assets/textures/Luigi-mini/jump/luigi.png");
    TextureManager::Load("luigi_mini_slide", "assets/textures/Luigi-mini/slide/luigi.png");
    TextureManager::Load("luigi_mini_die", "assets/textures/Luigi-mini/die/luigi.png");
    TextureManager::Load("luigi_mini_sit", "assets/textures/Luigi/sit/luigi.png"); // fallback

    // Items
    TextureManager::Load("mushroom", "assets/textures/mushroom/mushroom.png");

    // UI
    TextureManager::Load("gameover_img", "assets/textures/gameover.png");

    // Initialize player
    if (player_->GetPosition().x == 0 && player_->GetPosition().y == 0) {
        Vector2 spawn = tileMap.GetPlayerSpawn();
        Vector2 spawnPos = { spawn.x > 0 ? spawn.x : 100, spawn.y > 0 ? spawn.y : 300 };
        player_->SetPosition(spawnPos);
    }
    if (player_->IsSmall()) {
        player_->SetSize({Global::MINI_PLAYER_WIDTH * Global::GAME_SCALE, Global::MINI_PLAYER_HEIGHT * Global::GAME_SCALE}); 
    } else {
        player_->SetSize({Global::SUPER_PLAYER_WIDTH * Global::GAME_SCALE, Global::SUPER_PLAYER_HEIGHT * Global::GAME_SCALE}); 
    }
    player_->SyncPhysicsBody();
    
    // Extract map collision rects and pass to player
    mapCollisionRects = tileMap.GetCollisionRects();
    player_->SetCollisionBlocks(&mapCollisionRects);

    // Initialize enemy
    goomba_.SetPosition({ 500, 400 });
    goomba_.SetSize({ Global::TILE_SIZE * Global::GAME_SCALE, Global::TILE_SIZE * Global::GAME_SCALE });
    goomba_.SetPlayerBody(&player_->GetPhysicsBody());
    goomba_.SetCollisionBlocks(&mapCollisionRects);

    // Initialize mushroom
    mushroom_.SetPosition({ 300, 400 });
    mushroom_.SetCollisionBlocks(&mapCollisionRects);

    // Initialize camera
    view = View(16.0f, (float)tileMap.GetTileSize());
    view.Init((float)tileMap.GetPixelWidth(), (float)tileMap.GetPixelHeight());

    // Auto-save logic
    // We can just save current state.
    SaveData data;
    data.levelId = currentLevel.GetLevelNumber();
    data.playerX = player_->GetPosition().x;
    data.playerY = player_->GetPosition().y;
    data.score = score;
    data.timeLeft = timeLeft;
    SaveManager::SaveGame("auto_save", data);
    Global::hasSaveGame = true;
}

void GameplayState::Update(float deltaTime) {
    // Check Pause (ESC)
    if (IsKeyPressed(Global::keys.pause)) {
        Global::gameStateManager->PushState(std::make_unique<PauseMenuState>(this));
    }

    if (isGameOver || isGameWon) {
        if (IsKeyPressed(KEY_ENTER)) {
            Global::gameStateManager->PopState(); // Or go to Game Over screen
        }
        return;
    }

    timeLeft -= deltaTime;
    if (timeLeft <= 0) {
        timeLeft = 0;
        isGameOver = true;
    }

    player_->Update(deltaTime);

    // Update the goomba (chase AI + physics + collisions)
    if (goomba_.IsActive()) {
        goomba_.Update(deltaTime);
    }

    if (mushroom_.IsActive()) {
        mushroom_.Update(deltaTime);
    }

    // Entity interaction: player vs goomba/mushroom
    if (!player_->IsDead()) {
        if (goomba_.IsActive() && player_->Overlaps(goomba_)) {
            player_->InteractWith(goomba_);
        }
        if (mushroom_.IsActive() && player_->Overlaps(mushroom_)) {
            player_->InteractWith(mushroom_);
        }
    }

    // Handle player death (Game Over when falling off map)
    if (player_->IsDead()) {
        if (player_->GetPosition().y > tileMap.GetBorderBottom() + 100) {
            isGameOver = true;
        }
    }

    view.Update(player_->GetPosition().x, player_->GetPosition().y);
}

void GameplayState::Draw() {
    ClearBackground(tileMap.GetBackgroundColor());

    view.BeginDraw();

    tileMap.Draw(view.GetWorldLeft(), view.GetWorldTop(), view.GetRawCamera().zoom);

    // If you want to manually draw blocks using view, you can do so here:
    // view.DrawBlock(0, 0, 16, 16, RED);

    player_->Draw();
    if (goomba_.IsActive()) {
        goomba_.Draw();
    }
    if (mushroom_.IsActive()) {
        mushroom_.Draw();
    }

    view.EndDraw();

    // Draw HUD
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();
    int fontSize = (int)(sh * 0.035f);

    DrawText("MARIO", (int)(sw * 0.05f), (int)(sh * 0.03f), fontSize, WHITE);
    DrawText(TextFormat("%06i", score), (int)(sw * 0.05f), (int)(sh * 0.03f + fontSize + 4), fontSize, WHITE);

    const char* worldLabel = "WORLD";
    int worldX = (int)((sw - MeasureText(worldLabel, fontSize)) * 0.5f);
    DrawText(worldLabel, worldX, (int)(sh * 0.03f), fontSize, WHITE);
    
    int levelW = MeasureText(currentLevel.GetDisplayName().c_str(), fontSize);
    DrawText(currentLevel.GetDisplayName().c_str(), (int)(worldX + MeasureText(worldLabel, fontSize)/2 - levelW/2), (int)(sh * 0.03f + fontSize + 4), fontSize, WHITE);

    const char* timeLabel = "TIME";
    int timeX = (int)(sw - MeasureText(timeLabel, fontSize) - sw * 0.05f);
    DrawText(timeLabel, timeX, (int)(sh * 0.03f), fontSize, WHITE);
    DrawText(TextFormat("%03i", (int)timeLeft), timeX + 15, (int)(sh * 0.03f + fontSize + 4), fontSize, WHITE);

    if (isGameOver) {
        DrawRectangle(0, 0, sw, sh, Color{0, 0, 0, 150});
        
        int overSize = (int)(sh * 0.15f); // Increased size
        Vector2 goSize = MeasureTextEx(Global::titleFont, "GAME OVER", overSize, 1.0f);
        DrawTextEx(Global::titleFont, "GAME OVER", { (sw - goSize.x) / 2, sh / 2 - goSize.y }, overSize, 1.0f, RED);
        
        const char* prompt = "Press ENTER to return";
        int promptSize = (int)(sh * 0.05f);
        Vector2 pSize = MeasureTextEx(Global::baseFont, prompt, promptSize, 1.0f);
        DrawTextEx(Global::baseFont, prompt, { (sw - pSize.x) / 2, sh / 2 + 50 }, promptSize, 1.0f, WHITE);
    }

}

void GameplayState::Cleanup() {}
