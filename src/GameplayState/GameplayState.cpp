#include "GameplayState.h"
#include "raylib.h"
#include "TextureManager/TextureManager.h"
#include "Global/Global.h"
#include "physics/PhysicsEngine.h"
#include "physics/CollisionSystem.h"
#include "physics/ProximityAI.h"
#include "SaveManager/SaveManager.h"
#include "MainMenu/PauseMenuState/PauseMenuState.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Specific/Mario/Mario.h"
#include "Game_Objects/Derived_Objects/Playable_Characters/Specific/Luigi/Luigi.h"
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

void GameplayState::SetSandboxMode(const std::vector<std::vector<SandboxCellData>>& grid) {
    isSandboxMode_ = true;
    sandboxGrid_ = grid;
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

    data.isSandboxMode = isSandboxMode_;
    if (isSandboxMode_) {
        data.sandboxGrid = sandboxGrid_;
    }
    return data;
}

void GameplayState::Initialize() {
    // Load the tilemap based on sandbox mode or level selection
    if (isSandboxMode_) {
        if (!tileMap.LoadFromSandbox(sandboxGrid_)) {
            std::cerr << "Failed to load custom sandbox map" << std::endl;
        }
    } else {
        std::string ldtkPath = "assets/maps/maps.ldtk";
        if (!tileMap.LoadFromLdtk(ldtkPath, currentLevel.GetLdtkLevelId())) {
            std::cerr << "Failed to load tilemap for level: " << currentLevel.GetLdtkLevelId() << std::endl;
        }
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

    // Load Goomba, Koopa and Coin textures
    TextureManager::Load("goomba_texture", "assets/textures/enemies-3.png"); // legacy (sandbox preview)
    TextureManager::Load("goomba_walk",    "assets/textures/Goomba/walk/enemies.png");
    TextureManager::Load("goomba_dead",    "assets/textures/Goomba/dead/enemies.png");
    TextureManager::Load("koopa_walk",     "assets/textures/Koopa/walk/enemies.png");
    TextureManager::Load("koopa_hide",     "assets/textures/Koopa/hide/enemies.png");
    TextureManager::Load("buzzy_walk",    "assets/textures/BuzzyBeetle/walk/enemies.png");
    TextureManager::Load("buzzy_flipped", "assets/textures/BuzzyBeetle/flipped/enemies.png");
    TextureManager::Load("coin", "assets/textures/coin/coin.png");
    TextureManager::Load("mushroom", "assets/textures/Items/items.png");
    TextureManager::Load("luckyblock", "assets/textures/Luckyblock/luckyblock.png");

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
    
    // (We will append Luckyblock rects after they are initialized below)

    // Initialize enemies and coins
    goombas_.clear();
    koopas_.clear();
    buzzyBeetles_.clear();
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
                } else if (cell.type == 5) {
                    auto k = std::make_unique<KoopaShell>();
                    // Normal KoopaShell size is 16x24, same adjustment as the normal-level spawn path.
                    k->SetPosition({ (float)(c * tileMap.GetTileSize()), (float)(r * tileMap.GetTileSize()) - 8.0f * Global::GAME_SCALE });
                    k->SetSize({ 16.0f * Global::GAME_SCALE, 24.0f * Global::GAME_SCALE });
                    k->SetPlayerBody(&player_->GetPhysicsBody());
                    k->SetCollisionGrid(&tileMap.GetBlockGrid());
                    koopas_.push_back(std::move(k));
                } else if (cell.type == 6) {
                    auto b = std::make_unique<BuzzyBeetle>();
                    b->SetPosition({ (float)(c * tileMap.GetTileSize()), (float)(r * tileMap.GetTileSize()) });
                    b->SetSize({ Global::TILE_SIZE * Global::GAME_SCALE, Global::TILE_SIZE * Global::GAME_SCALE });
                    b->SetPlayerBody(&player_->GetPhysicsBody());
                    b->SetCollisionGrid(&tileMap.GetBlockGrid());
                    buzzyBeetles_.push_back(std::move(b));
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
            } else if (ent.id == "BuzzyBeetle") {
                auto b = std::make_unique<BuzzyBeetle>();
                b->SetPosition(ent.position);
                b->SetSize({ Global::TILE_SIZE * Global::GAME_SCALE, Global::TILE_SIZE * Global::GAME_SCALE });
                b->SetPlayerBody(&player_->GetPhysicsBody());
                b->SetCollisionGrid(&tileMap.GetBlockGrid());
                buzzyBeetles_.push_back(std::move(b));
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

    // Initialize camera
    view = View(16.0f, (float)tileMap.GetTileSize());
    view.Init((float)tileMap.GetPixelWidth(), (float)tileMap.GetPixelHeight());

    // Auto-save logic
    if (!isSandboxMode_) {
        SaveData data;
        data.levelId = currentLevel.GetLevelNumber();
        data.playerX = player_->GetPosition().x;
        data.playerY = player_->GetPosition().y;
        data.score = score;
        data.timeLeft = timeLeft;
        SaveManager::SaveGame("auto_save", data);
        Global::hasSaveGame = true;
    }
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

    // Update Goombas (including dying ones — they run their own timer)
    for (auto& g : goombas_) {
        if (g->IsActive()) {
            g->Update(deltaTime);
        }
    }
    
    // Update Koopas
    for (auto& k : koopas_) {
        if (k->IsActive()) {
            k->Update(deltaTime);
        }
    }

    // Update Buzzy Beetle
    for (auto& b : buzzyBeetles_) {
        if (b->IsActive()) {
            b->Update(deltaTime);
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

    if (mushroom_.IsActive()) {
        mushroom_.Update(deltaTime);
    }

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
                bool wasDying = g->IsDying();
                k->InteractWith(*g);
                if (!wasDying && g->IsDying()) score += 100;
            }
            }
        }
    }

    // Entity interaction: Koopa vs Koopa
    for (size_t i = 0; i < koopas_.size(); ++i) {
        auto& kA = koopas_[i];
        if (!kA->IsActive() || kA->GetState() != KoopaShellState::Sliding) continue;
        for (size_t j = i + 1; j < koopas_.size(); ++j) {
            auto& kB = koopas_[j];
            if (!kB->IsActive()) continue;
            if (kA->Overlaps(*kB)) {
                kA->InteractWith(*kB);
            }
        }
    }

    for (auto& k : koopas_) {
        if (k->IsActive() && k->GetState() == KoopaShellState::Sliding) {
            for (auto& b : buzzyBeetles_) {
                if (b->IsActive() && !b->IsDefeated() && k->Overlaps(*b)) {
                    bool wasDefeated = b->IsDefeated();
                    k->InteractWith(*b);
                    if (!wasDefeated && b->IsDefeated()) score += 100;
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

    // Draw interactive blocks
    tileMap.GetBlockGrid().Draw();

    player_->Draw();
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
    for (auto& b : buzzyBeetles_) {
        if (b->IsActive()) {
            b->Draw();
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

void GameplayState::Cleanup() {
    goombas_.clear();
    koopas_.clear();
    buzzyBeetles_.clear();
    coins_.clear();
}
