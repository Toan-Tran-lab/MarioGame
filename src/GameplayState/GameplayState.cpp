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
#include <algorithm>

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
    TextureManager::Load("mario_pose", "assets/textures/Mario/pose/mario.png");
    TextureManager::Load("mario_walk", "assets/textures/Mario/walk/mario.png");
    TextureManager::Load("mario_jump", "assets/textures/Mario/jump/mario.png");
    TextureManager::Load("mario_slide", "assets/textures/Mario/slide/mario.png");

    TextureManager::Load("luigi_pose", "assets/textures/Luigi/pose/luigi.png");
    TextureManager::Load("luigi_walk", "assets/textures/Luigi/walk/luigi.png");
    TextureManager::Load("luigi_jump", "assets/textures/Luigi/jump/luigi.png");
    TextureManager::Load("luigi_slide", "assets/textures/Luigi/slide/luigi.png");
<<<<<<< HEAD
=======
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

    // Load Goomba, Koopa and Coin textures
    TextureManager::Load("goomba_texture", "assets/textures/enemies-3.png"); // legacy (sandbox preview)
    TextureManager::Load("goomba_walk",    "assets/textures/Goomba/walk/enemies.png");
    TextureManager::Load("goomba_dead",    "assets/textures/Goomba/dead/enemies.png");
    TextureManager::Load("koopa_walk",     "assets/textures/Koopa/walk/enemies.png");
    TextureManager::Load("koopa_hide",     "assets/textures/Koopa/hide/enemies.png");
    TextureManager::Load("coin", "assets/textures/coin/coin.png");
    TextureManager::Load("mushroom", "assets/textures/Items/items.png");
    TextureManager::Load("luckyblock", "assets/textures/Luckyblock/luckyblock.png");
>>>>>>> c82ad2234b5ca71bdccf2f742362e3fad6a65b7c

    // Initialize player
    if (player_->GetPosition().x == 0 && player_->GetPosition().y == 0) {
        Vector2 spawn = tileMap.GetPlayerSpawn();
        Vector2 spawnPos = { spawn.x > 0 ? spawn.x : 100, spawn.y > 0 ? spawn.y : 300 };
        player_->SetPosition(spawnPos);
    }
    float scl = 2;
    player_->SetSize({16 * scl, 30 * scl}); // Use SetSize so it persists into physics body
    player_->SyncPhysicsBody();
    
    // (We will append Luckyblock rects after they are initialized below)

<<<<<<< HEAD
    // Initialize enemy
    goomba_.SetPosition({ 500, 400 });
    goomba_.SetSize({ 32, 32 });
    goomba_.SetPlayerBody(&player_->GetPhysicsBody());
    goomba_.SetCollisionBlocks(&mapCollisionRects);


=======
    // Initialize enemies and coins
    goombas_.clear();
    koopas_.clear();
    coins_.clear();

    if (isSandboxMode_) {
        // Spawn Goombas and Coins based on Sandbox coordinates
        for (int r = 0; r < tileMap.GetMapHeight(); ++r) {
            for (int c = 0; c < tileMap.GetMapWidth(); ++c) {
                if (c >= (int)sandboxGrid_[r].size()) continue;
                const auto& cell = sandboxGrid_[r][c];
                if (cell.type == 3) {
                    auto g = std::make_unique<Goomba>();
                    g->SetPosition({ (float)(c * tileMap.GetTileSize()), (float)(r * tileMap.GetTileSize()) });
                    g->SetSize({ Global::TILE_SIZE * Global::GAME_SCALE, Global::TILE_SIZE * Global::GAME_SCALE });
                    g->SetPlayerBody(&player_->GetPhysicsBody());
                    g->SetCollisionGrid(&tileMap.GetBlockGrid());
                    goombas_.push_back(std::move(g));
                } else if (cell.type == 4) {
                    auto c_coin = std::make_unique<Coin>();
                    c_coin->SetPosition({ (float)(c * tileMap.GetTileSize()), (float)(r * tileMap.GetTileSize()) });
                    c_coin->SetSize({ 48.0f, 48.0f });
                    coins_.push_back(std::move(c_coin));
                }
            }
        }
    } else {
        // Normal level entity spawn from TileMap
        const auto& entities = tileMap.GetEntities();
        for (const auto& ent : entities) {
            if (ent.id == "Goomba") {
                auto g = std::make_unique<Goomba>();
                g->SetPosition(ent.position);
                g->SetSize({ Global::TILE_SIZE * Global::GAME_SCALE, Global::TILE_SIZE * Global::GAME_SCALE });
                g->SetPlayerBody(&player_->GetPhysicsBody());
                g->SetCollisionGrid(&tileMap.GetBlockGrid());
                goombas_.push_back(std::move(g));
            } else if (ent.id == "KoopaShell") {
                auto k = std::make_unique<KoopaShell>();
                // Normal KoopaShell size is 16x24. We scale it.
                // Notice: position Y might need adjustment because its height is 24, not 16.
                k->SetPosition({ ent.position.x, ent.position.y - 8.0f * Global::GAME_SCALE });
                k->SetSize({ 16.0f * Global::GAME_SCALE, 24.0f * Global::GAME_SCALE });
                k->SetPlayerBody(&player_->GetPhysicsBody());
                k->SetCollisionGrid(&tileMap.GetBlockGrid());
                koopas_.push_back(std::move(k));
            } else if (ent.id == "Coin") {
                auto c_coin = std::make_unique<Coin>();
                c_coin->SetPosition(ent.position);
                c_coin->SetSize({ 16.0f * Global::GAME_SCALE, 16.0f * Global::GAME_SCALE });
                coins_.push_back(std::move(c_coin));
            } else if (ent.id == "Luckyblock") {
                auto block = std::make_unique<Luckyblock>();
                block->SetPosition(ent.position);
                int col = (int)(ent.position.x / tileMap.GetTileSize());
                int row = (int)(ent.position.y / tileMap.GetTileSize());
                tileMap.GetBlockGrid().SetBlock(col, row, std::move(block));
            }
        }
    }

    // Now pass the block grid to the player
    player_->SetCollisionGrid(&tileMap.GetBlockGrid());

    // Initialize mushroom
    mushroom_.SetPosition({ 300, 400 });
    mushroom_.SetCollisionGrid(&tileMap.GetBlockGrid());
>>>>>>> c82ad2234b5ca71bdccf2f742362e3fad6a65b7c

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

<<<<<<< HEAD
    // Update the goomba (chase AI + physics + collisions)
    if (goomba_.IsActive()) {
        goomba_.Update(deltaTime);
=======
    // Update Goombas (including dying ones — they run their own timer)
    for (auto& g : goombas_) {
        if (g->IsActive()) {
            g->Update(deltaTime);
        }
>>>>>>> c82ad2234b5ca71bdccf2f742362e3fad6a65b7c
    }
    
    // Update Koopas
    for (auto& k : koopas_) {
        if (k->IsActive()) {
            k->Update(deltaTime);
        }
    }

    // Update block grid
    tileMap.GetBlockGrid().Update(deltaTime);

    // Update debris particles
    for (auto& d : debrisList_) {
        d.Update(deltaTime);
    }
    debrisList_.erase(std::remove_if(debrisList_.begin(), debrisList_.end(),
        [](const DebrisPiece& d) { return !d.active; }), debrisList_.end());

    if (player_->CanHitBlock() && player_->GetPhysicsBody().hitCeiling) {
        Rectangle hitRect = player_->GetPhysicsBody().hitCeilingRect;
        float headX = player_->GetPosition().x + player_->GetSize().x / 2.0f;
        
        // Find which block we hit in the grid based on Mario's head X and the ceiling Y
        int col = (int)(headX / tileMap.GetTileSize());
        int row = (int)(hitRect.y / tileMap.GetTileSize());
        Block* block = tileMap.GetBlockGrid().GetBlock(col, row);
        
        if (block) {
            // Check if Mario is directly under this specific block
            if (headX >= hitRect.x && headX <= hitRect.x + hitRect.width) {
                bool isSmall = player_->IsSmall();
                
                if (block->IsLucky()) {
                    // Active Luckyblock (has coin): spawns item/coin and bounces (cannot break on first hit)
                    if (block->Bump()) {
                        auto c_coin = std::make_unique<Coin>();
                        c_coin->SetPosition({ hitRect.x, hitRect.y - 16.0f * Global::GAME_SCALE });
                        c_coin->SetSize({ 16.0f * Global::GAME_SCALE, 16.0f * Global::GAME_SCALE });
                        c_coin->SetPopping(true, -350.0f);
                        coins_.push_back(std::move(c_coin));
                        score += 100;
                    }
                    player_->SetCanHitBlock(false);
                } else {
                    // Non-lucky block (regular brick or empty luckyblock)
                    if (isSmall) {
                        // Mini Mario: Bumps the block (bounces up and down)
                        block->Bump();
                    } else {
                        // Super Mario: Breaks the block into 4 debris fragments and destroys it
                        std::string texKey;
                        Rectangle srcRect = { 0, 0, 16, 16 };
                        bool foundTile = tileMap.RemoveTileAt(col, row, texKey, srcRect);
                        
                        Rectangle blockWorldRect = {
                            (float)(col * tileMap.GetTileSize()),
                            (float)(row * tileMap.GetTileSize()),
                            (float)tileMap.GetTileSize(),
                            (float)tileMap.GetTileSize()
                        };
                        
                        if (foundTile) {
                            SpawnBlockDebris(debrisList_, blockWorldRect, texKey, srcRect);
                        } else {
                            SpawnBlockDebris(debrisList_, blockWorldRect, "luckyblock", { 64.0f, 0.0f, 16.0f, 16.0f });
                        }

                        // Remove block from collision grid
                        tileMap.GetBlockGrid().SetBlock(col, row, nullptr);
                        score += 50;
                    }
                    player_->SetCanHitBlock(false);
                }
            }
        }
    }



    // Entity interaction: player vs goomba
    if (goomba_.IsActive() && player_->Overlaps(goomba_)) {
        player_->InteractWith(goomba_);
    }

<<<<<<< HEAD
    // Handle player death (respawn at spawn point)
=======
    // Entity interaction: player vs goombas/koopas/mushroom
    if (!player_->IsDead()) {
        for (auto& g : goombas_) {
            // Only interact with alive, non-dying goombas
            if (g->IsActive() && !g->IsDying() && player_->Overlaps(*g)) {
                bool wasDying = g->IsDying(); // always false here, kept for clarity
                player_->InteractWith(*g);
                // If the interaction caused a stomp (Goomba just entered Dying)
                if (!wasDying && g->IsDying()) {
                    score += 100;
                }
            }
        }
        for (auto& k : koopas_) {
            if (k->IsActive() && player_->Overlaps(*k)) {
                KoopaShellState prevState = k->GetState();
                player_->InteractWith(*k);
                
                // If stomped from Walking -> Hiding or Sliding -> Hiding
                if (prevState != KoopaShellState::Hiding && k->GetState() == KoopaShellState::Hiding) {
                    score += 100;
                }
            }
        }
        if (mushroom_.IsActive() && player_->Overlaps(mushroom_)) {
            player_->InteractWith(mushroom_);
        }
    }

    // Entity interaction: Koopa vs Goomba
    for (auto& k : koopas_) {
        if (k->IsActive() && k->GetState() == KoopaShellState::Sliding) {
            for (auto& g : goombas_) {
                if (g->IsActive() && !g->IsDying() && k->Overlaps(*g)) {
                    g->Stomp();
                    score += 100; // Bonus points for shell kill
                }
            }
        }
    }

    // Update and collect Coins
    for (auto& coin : coins_) {
        if (coin->IsActive()) {
            coin->Update(deltaTime);
            Rectangle pRect = player_->GetPhysicsBody().GetRect();
            Rectangle cRect = { coin->GetPosition().x, coin->GetPosition().y, coin->GetSize().x, coin->GetSize().y };
            if (CheckCollisionRecs(pRect, cRect)) {
                coin->SetActive(false);
                score += 100;
            }
        }
    }

    // Handle player death (Game Over when falling off map)
>>>>>>> c82ad2234b5ca71bdccf2f742362e3fad6a65b7c
    if (player_->IsDead()) {
        player_->SetDead(false);
        Vector2 spawn = tileMap.GetPlayerSpawn();
        Vector2 spawnPos = { spawn.x > 0 ? spawn.x : 100, spawn.y > 0 ? spawn.y : 300 };
        player_->SetPosition(spawnPos);
        player_->SyncPhysicsBody();
    }

    view.Update(player_->GetPosition().x, player_->GetPosition().y);
}

void GameplayState::Draw() {
    ClearBackground(tileMap.GetBackgroundColor());

    view.BeginDraw();

    tileMap.Draw(view.GetWorldLeft(), view.GetWorldTop(), view.GetRawCamera().zoom);

    // Draw interactive blocks
    tileMap.GetBlockGrid().Draw();

    player_->Draw();
<<<<<<< HEAD
    if (goomba_.IsActive()) {
        goomba_.Draw();
    }


=======
    for (auto& g : goombas_) {
        // Draw alive AND dying goombas (dying ones show squish + "+100" popup)
        if (g->IsActive()) {
            g->Draw();
        }
    }
    for (auto& k : koopas_) {
        if (k->IsActive()) {
            k->Draw();
        }
    }
    for (auto& coin : coins_) {
        if (coin->IsActive()) {
            coin->Draw();
        }
    }
    if (mushroom_.IsActive()) {
        mushroom_.Draw();
    }

    for (const auto& d : debrisList_) {
        d.Draw();
    }
>>>>>>> c82ad2234b5ca71bdccf2f742362e3fad6a65b7c

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
    DrawText(currentLevel.GetDisplayName().c_str(), worldX - 30, (int)(sh * 0.03f + fontSize + 4), fontSize, WHITE);

    const char* timeLabel = "TIME";
    int timeX = (int)(sw - MeasureText(timeLabel, fontSize) - sw * 0.05f);
    DrawText(timeLabel, timeX, (int)(sh * 0.03f), fontSize, WHITE);
    DrawText(TextFormat("%03i", (int)timeLeft), timeX + 15, (int)(sh * 0.03f + fontSize + 4), fontSize, WHITE);

    if (isGameOver) {
        DrawRectangle(0, 0, sw, sh, Color{0, 0, 0, 150});
        int overSize = (int)(sh * 0.1f);
        int textW = MeasureText("GAME OVER", overSize);
        DrawText("GAME OVER", (sw - textW) / 2, sh / 2 - overSize, overSize, RED);
        DrawText("Press ENTER to return", (sw - MeasureText("Press ENTER to return", 20)) / 2, sh / 2 + 50, 20, WHITE);
    }

}

void GameplayState::Cleanup() {}
